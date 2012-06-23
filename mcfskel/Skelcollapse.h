#pragma once
#pragma GCC diagnostic ignored "-Wwrite-strings"

#include <QDir>
#include "interfaces/SurfaceMeshModelPlugins.h"
#include "StarlabDrawArea.h"
#include "SurfaceMeshHelper.h"
#include "Logfile.h"

typedef QList<Surface_mesh::Vertex> VertexList;
typedef Surface_mesh::Vertex_property<VertexList> VertexListVertexProperty;

#ifdef USE_MATLAB
    const bool use_matlab = true;
#else
    const bool use_matlab = false;
#endif


class Skelcollapse : public FilterPlugin{
    Q_OBJECT
    Q_INTERFACES(FilterPlugin)
    
public:
    virtual QString name() { return "MCF Skeletonization"; }
    virtual QString description() { return "Performs Skeletonization by controlled Mean Curvature Flow"; }
    virtual QKeySequence shortcut(){ return QKeySequence(Qt::CTRL + Qt::Key_L); }
    
private:
    /// @{ algorithm parameters
        Scalar omega_L_0;
        Scalar omega_H_0;
        Scalar omega_P_0;
        Scalar edgelength_TH;
        Scalar zero_TH;        
    /// @} 
        
    /// @{ algorithm internal data
        VertexListVertexProperty corrs;            
        SurfaceMeshModel*     mesh;
        StarlabDrawArea*      drawArea;
        Vector3VertexProperty points;
        Vector3VertexProperty poles;
        ScalarVertexProperty  omega_H;
        ScalarVertexProperty  omega_L;
        ScalarVertexProperty  omega_P;
        BoolVertexProperty    vissplit;
        BoolVertexProperty    visfixed;
        bool                  isInitialized;
    /// @}
        
public:
    void initParameters(Document* document, RichParameterSet* parameters, StarlabDrawArea* drawArea){
        SurfaceMeshModel* mesh = qobject_cast<SurfaceMeshModel*>(document->selectedModel());
        Scalar scale = 0.002*mesh->getBoundingBox().size().length();
        parameters->addParam(new RichFloat("omega_L_0",1));
        parameters->addParam(new RichFloat("omega_H_0",use_matlab?20:0.1));
        parameters->addParam(new RichFloat("omega_P_0",use_matlab?40:0.2));
        parameters->addParam(new RichFloat("edgelength_TH",scale));
        parameters->addParam(new RichFloat("alpha",0.15));
        parameters->addParam(new RichFloat("zero_TH",1e-10));
        
        /// Add a transparent copy of the model, must be done only when the parameter window
        /// is open (a.k.a. on first iteration)
        SurfaceMeshModel* copy = new SurfaceMeshModel(mesh->path,"original");
        copy->read( mesh->path.toStdString() );
        document->addModel(copy);
        drawArea->initRendering();
        copy->renderer()->setRenderMode("Transparent"); 
    }
    void setupFilter(Document *document, RichParameterSet *pars, StarlabDrawArea *drawArea){
        this->mesh     = qobject_cast<SurfaceMeshModel*>( document->selectedModel() );
        this->drawArea = drawArea;
                
        { /// Retrieve parameters
            omega_L_0     = pars->getFloat("omega_L_0"); 
            omega_H_0     = pars->getFloat("omega_H_0");
            omega_P_0     = pars->getFloat("omega_P_0");
            edgelength_TH = pars->getFloat("edgelength_TH");
            zero_TH       = pars->getFloat("zero_TH");
        }
        
        { /// Retrieve properties
            SurfaceMeshHelper helper(mesh);
            points        = helper.getVector3VertexProperty(VPOINT);
            poles         = helper.getVector3VertexProperty("v:pole");
            omega_H       = mesh->vertex_property<Scalar>("v:omega_H",omega_H_0);
            omega_L       = mesh->vertex_property<Scalar>("v:omega_L",omega_L_0);
            omega_P       = mesh->vertex_property<Scalar>("v:omega_P",omega_P_0);
            vissplit      = mesh->vertex_property<bool>("v:issplit",false);
            visfixed      = mesh->vertex_property<bool>("v:isfixed",false);
            isInitialized = mesh->property("isInitialized").toBool();
            corrs         = mesh->vertex_property<VertexList>("v:corrs");
        }
    }

    void applyFilter(Document* document, RichParameterSet* pars, StarlabDrawArea* drawArea){
        drawArea->deleteAllRenderObjects();
        setupFilter(document,pars,drawArea);
        
        if(!isInitialized){
            /// Setup LOG file
            tictocreset("log.txt");

            /// Every vertex initially corresponds to itself
            foreach(Vertex v, mesh->vertices())
                corrs[v].push_back(v);
        
            /// Change name/path of the model
            QFileInfo fi(mesh->path);
            QString basename = fi.baseName();
            QString currpath = fi.dir().path();
            mesh->path = currpath+"/"+basename+"_ckel.off";
        }
        
        /// Run a single iteration (for now...)
        algorithm_iteration();
        
        /// Tell the model this is not its first iteration
        mesh->setProperty("isInitialized",true);
    }

    void algorithm_iteration(){  
        logme("----------- ITERATION ----------");
        tic("Geometry Contraction"); contractGeometry();   toc();
        tic("Constraints Update");   updateConstraints();  toc();
        tic("Update Topology");      updateTopology();     toc();        
        tic("Detect Degeneracies");  detectDegeneracies(); toc();

        /// Highlight fixed vertices
        foreach(Vertex v, mesh->vertices()){
            if( visfixed[v] )
                drawArea->drawPoint(points[v],3,Qt::red);        
        }    
    }
    
    void updateConstraints();
    void contractGeometry();
    void detectDegeneracies();
    void updateTopology();
};
