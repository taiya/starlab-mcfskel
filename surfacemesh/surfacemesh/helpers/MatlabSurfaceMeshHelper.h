#pragma once
#include "engine.h"
#include "SurfaceMeshHelper.h"

class MatlabEngineHelper{
public:
    static Engine* engine;

    static const int BUFSIZE=256;
    char buffer[BUFSIZE];

    /// Default connects to localhost
    MatlabEngineHelper(QString host=""){
        if(engine==NULL)
            engine = engOpen(host.toStdString().c_str());
        if(!engine) 
            throw StarlabException("Impossible to connect to matlab server");    
        
        /// Setup output buffer
        engOutputBuffer(engine,buffer,BUFSIZE);
        
        /// Enable warnings (solver throws them)
        eval("warning on;");
        
        /// Hides Matlab window
        //engSetVisible(engine,false);
    }
    void shutdown(){
        int success = engClose(engine);
        if(!success) throw StarlabException("Impossible to terminate matlab server");
    }   
    void put(const char *var_name, const mxArray *ap){
        int failure = engPutVariable(engine, var_name, ap);
        if(failure) throw StarlabException("Failed to add %s", var_name);
    }
    void eval(const char *string){
        int retval = engEvalString(engine,string);
        if(buffer[0]!='\0') qDebug() << "Matlab says: " << buffer;
        if(retval) throw StarlabException("Command %s failed", string);
    }
    void check_for_warnings(){        
        engEvalString(engine,"wmessage=lastwarn();");
        engEvalString(engine,"lastwarn('');");
        mxArray* wm = engGetVariable(engine,"wmessage");
        int buflen = mxGetN(wm)*sizeof(mxChar)+1;
        /* Copy the string data into buf. */ 
        std::string buffer(buflen,'\0');
        mxGetString(wm, &buffer[0], buflen);        
        // >1 as empty string is '\0'
        if(buflen>1)
            throw StarlabException("SOLVER FAIL");
    }
    
    mxArray* get(const string name){
        mxArray* retval = engGetVariable(engine,name.c_str());
        if(!retval) throw StarlabException("Failure to extract: %s",name.c_str());
        return retval;
    }
};

/// Init statics
Engine* MatlabEngineHelper::engine = NULL;

/// Auto-conversion ot 1-indexed matlab
/// Set convert it to M-index
/// Get convert it to C-index
struct IndexMatrix{
    int n,m;
    double* data;
    
    IndexMatrix(const mxArray* mxa){
        data = mxGetPr(mxa);
        n = mxGetDimensions(mxa)[0];
        m = mxGetDimensions(mxa)[1];
    }
        
    // Set value at index (i,j) assumed in C-format
    // val is going to be convertex C-fomat => M-format
    void set(int i, int j, double val){
        data[ i + n*j ] = val+1;
    }
    
    // val C-format => M-format
    void set(int i, double val){
        data[ i ] = val+1;
    }
    
    // retrieves an index in C-format
    int get(int i, int j){
        return ((int) data[ i+n*j ])-1;
    }
};

struct ScalarMatrix{
    int n,m;
    double* data;
    
    // Constructor
    ScalarMatrix(const mxArray* mxa){
        data = mxGetPr(mxa);
        n = mxGetDimensions(mxa)[0];
        m = mxGetDimensions(mxa)[1];
    }
    void set(int i, double val){
        data[ i ] = val;
    }
};

struct ScalarVector{
    int n;
    double* data;
    
    ScalarVector(const mxArray* mxa){
        data = mxGetPr(mxa);
        n = mxGetNumberOfElements(mxa);
    }
    void set(int i, double val){
        Q_ASSERT(i>=0 && i<n);
        data[ i ] = val;
    }
    double get(int i){
        Q_ASSERT(i>=0 && i<n);
        return data[i];
    }
};

/// @todo rename to IndexCell
struct Cell{
    double* data;
    int numel;
    // Constructors
    Cell(){
        data = NULL;
        numel = 0;
    }
    Cell( mxArray* cell_mxa ){
        data = mxGetPr(cell_mxa);
        numel = mxGetDimensions(cell_mxa)[1];
    }
    int getItem(int i){
        mxAssert(i<numel,"error");
        //Converts to C-index
        return data[i]-1; 
    }
};

class MatlabSurfaceMeshHelper : public MatlabEngineHelper, public virtual SurfaceMeshHelper{
public:
    IndexVertexProperty vindex;
public:
    MatlabSurfaceMeshHelper(SurfaceMeshModel* mesh) : SurfaceMeshHelper(mesh){}

    IndexVertexProperty createVertexIndexes(const string name="v:index"){
        vindex = mesh->add_vertex_property<Index>(name,-1);
        Counter nv = mesh->n_vertices();
        mxArray* _nv = mxCreateDoubleScalar(nv);
        put("nv", _nv);
        
        /// Create indexes for mesh vertices
        vindex = mesh->vertex_property<Index>("v:index",-1);
        Index curr_vidx = 0;
        foreach(Vertex v, mesh->vertices())
            vindex[v] = curr_vidx++;
        return vindex;
    }    
    
    void meshVerticesToVariable(const char* varname){
        Vector3VertexProperty points = mesh->get_vertex_property<Vector3>(VPOINT);        
        if(!vindex) throw MissingPropertyException("v:index");
        mxArray* _x0 = mxCreateDoubleMatrix(mesh->n_vertices(),3,mxREAL);
        double* x0 = mxGetPr(_x0);
        Index nrows = mesh->n_vertices();
        foreach(Vertex v, mesh->vertices()){
            x0[ vindex[v] + 0*nrows ] = points[v].x();
            x0[ vindex[v] + 1*nrows ] = points[v].y();
            x0[ vindex[v] + 2*nrows ] = points[v].z();
        }
        put(varname, _x0);
    }
    
    /// @obsolete by "variableToVector3VertexProperty"
    void variableToMeshVertices(const char* varname){
        if(!vindex) throw MissingPropertyException("v:index");
        Vector3VertexProperty points = mesh->get_vertex_property<Vector3>(VPOINT);        
        mxArray* _var = get(varname);
        double* var = mxGetPr(_var);
        Index nrows = mesh->n_vertices();
        foreach(Vertex v, mesh->vertices()){
            points[v].x() = var[ vindex[v] + 0*nrows ];
            points[v].y() = var[ vindex[v] + 1*nrows ];
            points[v].z() = var[ vindex[v] + 2*nrows ];               
        }
    }

    void variableToVector3VertexProperty(const string varname, const string propname){
        Vector3VertexProperty property = defaultedVector3VertexProperty(propname,Vector3(0,0,0));
        mxArray* _var = get(varname);
        double* var = mxGetPr(_var);
        Index nrows = mesh->n_vertices();
        
        /// Make sure matlab arrays are Nx3
        if(mxGetM(_var)!=mesh->n_vertices()) throw StarlabException("MATLAB memory copy error");
        if(mxGetN(_var)!=3) throw StarlabException("MATLAB memory copy error");
            
        /// Now copy memory
        foreach(Vertex v, mesh->vertices()){
            property[v].x() = var[ vindex[v] + 0*nrows ];
            property[v].y() = var[ vindex[v] + 1*nrows ];
            property[v].z() = var[ vindex[v] + 2*nrows ];               
        }
    }
    
    void variableToVertexScalarProperty(const string varname, const string propname){
        ScalarVertexProperty prop = mesh->get_vertex_property<Scalar>(propname);
        if(!vindex) throw MissingPropertyException("v:index");
        if(!prop) throw MissingPropertyException(propname);
        mxArray* _var = get(varname);
        double* var = mxGetPr(_var);
        foreach(Vertex v, mesh->vertices())
            prop[v] = var[ vindex[v] ];
    }
};
