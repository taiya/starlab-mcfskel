#include "Skelcollapse.h"
#include "LaplacianHelper.h"
#include <QList>
#include <QDir>
#include "TopologyJanitor.h"
#include "starlab/plugins/surfacemesh_io_off/surfacemesh_io_helpers.h"

// #define USE_POLE_ANGLE
#ifdef USE_POLE_ANGLE
/// [0...1] weight, 
double angle_weight(Scalar alpha, Scalar TH_ALPHA=30.0){
    double TH_ALPHA_RAD = TH_ALPHA/180.0*3.1415;
    double sigma2 = ( TH_ALPHA_RAD / 2.5 ) * ( TH_ALPHA_RAD / 2.5 );
    double omega = exp( -pow(3.1415-alpha,2) / sigma2 );
    return omega;
}
#endif

/// Augments the setContraints with poles information
class PoleAttractorHelper : public MatlabMeanValueLaplacianHelper{
public:
    PoleAttractorHelper(SurfaceMeshModel* mesh) : SurfaceMeshHelper(mesh), LaplacianHelper(mesh), MatlabMeanValueLaplacianHelper(mesh){}
    void setConstraints(ScalarVertexProperty omega_H, ScalarVertexProperty omega_L, ScalarVertexProperty omega_P, Vector3VertexProperty poles){
        /// Do what was already there
        MatlabMeanValueLaplacianHelper::setConstraints(omega_H,omega_L);
        if(!omega_P) throw MissingPropertyException("Invalid");
        BoolVertexProperty vissplit = mesh->get_vertex_property<bool>("v:issplit");
        
        /// Initialize omega_P (poles constraints)
        {
            mxArray* _w = mxCreateDoubleMatrix(mesh->n_vertices(),1,mxREAL);
            double* w = mxGetPr(_w); 
            foreach(Vertex v, mesh->vertices())
                w[ vindex[v] ] = omega_P[v];
            put("omega_P", _w);            
        }
        
        /// Initialize p0 (poles positions)
        {
            mxArray* _p0 = mxCreateDoubleMatrix(mesh->n_vertices(),3,mxREAL);
            double* p0 = mxGetPr(_p0);
            Index nrows = mesh->n_vertices();
            foreach(Vertex v, mesh->vertices()){
                if(vissplit[v]) continue;
                p0[ vindex[v] + 0*nrows ] = poles[v].x();
                p0[ vindex[v] + 1*nrows ] = poles[v].y();
                p0[ vindex[v] + 2*nrows ] = poles[v].z();
            }
            put("p0", _p0);
        }
    }
    void solve(){
        eval("save('/Users/ata2/Developer/skelcollapse/poles.mat')");        
        eval("nv = size(L,1);");
        eval("OMEGA_L = spdiags(omega_L,0,nv,nv);");
        eval("L = OMEGA_L * L';");
        eval("H = spdiags(omega_H, 0, nv, nv);");
        eval("P = spdiags(omega_P, 0, nv, nv);");
        eval("LHS = [L; H; P];");
        eval("RHS = [ zeros(nv,3) ; H*x0 ; P*p0];");
        eval("x = LHS \\ RHS;");
        check_for_warnings();
    }
}; 

class TopologyJanitor_ClosestPole : public TopologyJanitor{
public:
    TopologyJanitor_ClosestPole(SurfaceMeshModel* mesh) : SurfaceMeshHelper(mesh), TopologyJanitor(mesh){}
       
    /// @{ This collapse mode retains only the closest pole greedily
    virtual Counter collapser(Scalar edgelength_TH){
        Vector3VertexProperty points = mesh->get_vertex_property<Point>("v:point");
        Vector3VertexProperty poles = mesh->vertex_property<Vector3>("v:poles");
#ifdef USE_POLE_ANGLE
        ScalarVertexProperty pangle = mesh->vertex_property<Scalar>("v:pangle");
#endif        
        /// Keep the set of poles associated with the point
        typedef QList<Vector3> PoleList;
        typedef Surface_mesh::Vertex_property<PoleList> VSetVertexProperty;
        VSetVertexProperty pset = mesh->vertex_property<PoleList>("v:pset");
        
        Counter count=0;
        foreach(Edge e,mesh->edges()){
            Halfedge h = mesh->halfedge(e,0);
            if(mesh->edge_length(e)<edgelength_TH){
                if(!mesh->is_deleted(h) && mesh->is_collapse_ok(h)){
                    Vertex v0 = mesh->from_vertex(h);
                    Vertex v1 = mesh->to_vertex(h);
                    points[v1] = (points[v0]+points[v1])/2.0f;
                     
                    /// Find the closest pole
                    Vector3 pole0 = poles[v0];
                    Vector3 pole1 = poles[v1];
                    Scalar d0 = (pole0 - points[v1]).norm();
                    Scalar d1 = (pole1 - points[v1]).norm();
                    
                    /// Pick it
                    poles[v1] = (d0<d1) ? poles[v0] : poles[v1];
#ifdef USE_POLE_ANGLE
                    pangle[v1] = (d0<d1) ? pangle[v0] : pangle[v1];
#endif                            
                    /// And keep track of correspondences
                    pset[v1] += pset[v0];
                    
                    /// Perform collapse
                    mesh->collapse(h);
                    count++;
                }
            }
        }
        return count;
    }
    virtual Counter splitter(Scalar short_edge, Scalar TH_ALPHA /*110*/){
        Vector3VertexProperty points = mesh->get_vertex_property<Point>("v:point");
        Vector3VertexProperty poles  = mesh->get_vertex_property<Vector3>("v:poles");
#ifdef USE_POLE_ANGLE
        ScalarVertexProperty pangle = mesh->vertex_property<Scalar>("v:pangle");
#endif
        /// Keep track / decide which to split    
        TH_ALPHA *= (3.14/180);
        
        /// Store halfedge opposite angles
        ScalarHEdgeProperty halpha = cacheAngles(short_edge);
                        
        /// Splitting section
        Scalar numsplits=0;
        BoolVertexProperty vissplit = mesh->vertex_property<bool>("v:issplit",false);
        foreach(Edge e, mesh->edges()){
            Halfedge h0 = mesh->halfedge(e,0);
            Halfedge h1 = mesh->halfedge(e,1);
    
            /// Should a split take place?
            Scalar alpha_0 = halpha[ h0 ];
            Scalar alpha_1 = halpha[ h1 ];
            if(alpha_0<TH_ALPHA || alpha_1<TH_ALPHA) continue;
            
            /// Which side should I split?
            Vertex w0 = mesh->to_vertex( mesh->next_halfedge(h0) );
            Vertex w1 = mesh->to_vertex( mesh->next_halfedge(h1) );
            Vertex wsplitside = (alpha_0>alpha_1) ? w0 : w1;
            
            /// Project side vertex on edge
            Point p0 = points[mesh->vertex(e,0)];
            Point p1 = points[mesh->vertex(e,1)];
            Vector3 projector = (p1-p0).normalized();
            Vector3 projectee = points[wsplitside]-p0;
            Scalar t = dot(projector, projectee);
            
            Q_ASSERT(!isnan(t));
            Vector3 newpos = p0 + t*projector;
            
            /// Perform the split at the desired location
            Vertex vnew = mesh->split(e,newpos);
            
            /// Also project the pole
            Vector3 pole0 = poles[mesh->vertex(e,0)];
            Vector3 pole1 = poles[mesh->vertex(e,1)];            
            Vector3 p_projector = (pole1-pole0).normalized();
            poles[vnew] = pole0 + t*p_projector; 
#ifdef USE_POLE_ANGLE
            pangle[vnew] = 0; /// NO EFFECT
#endif     
            /// And mark it as a split
            vissplit[vnew] = true;
            numsplits++;
        }
        return numsplits;
    }
};

void Skelcollapse::initParameters(Document *document, RichParameterSet *parameters, StarlabDrawArea *drawArea){
    SurfaceMeshModel* model = qobject_cast<SurfaceMeshModel*>(document->selectedModel());
    mesh = model;

    /// Add a transparent copy of the model
    {
        SurfaceMeshModel* copy = new SurfaceMeshModel(mesh->path,"original");
        copy->read( mesh->path.toStdString() );
        document->addModel(copy);
        drawArea->initRendering();
        copy->renderer()->setRenderMode("Transparent"); 
    }
        
    Scalar scale = 0.002*model->getBoundingBox().size().length();
    parameters->addParam(new RichFloat("omega_L_0",1));
    parameters->addParam(new RichFloat("omega_H_0",20));
    parameters->addParam(new RichFloat("omega_P_0",40));
    parameters->addParam(new RichFloat("edgelength_TH",scale));
    parameters->addParam(new RichFloat("alpha",0.15));
    parameters->addParam(new RichFloat("zero_TH",1e-10));
}

void Skelcollapse::algorithm(RichParameterSet* pars){
    if(firststep) mesh->renderer()->setRenderMode("Smooth"); 
    qDebug() << "==== Medial Contraction ===";
    qDebug() << "Step# : " << stepcount;
    
    static PoleAttractorHelper h(mesh);
    h.eval("lastwarn('');");
    Scalar omega_L_0 = pars->getFloat("omega_L_0"); 
    Scalar omega_H_0 = pars->getFloat("omega_H_0");
    Scalar omega_P_0 = pars->getFloat("omega_P_0");
    Scalar edgelength_TH = pars->getFloat("edgelength_TH");
    Scalar zero_TH = pars->getFloat("zero_TH");
    Scalar alpha = pars->getFloat("alpha");
        
    /// Compute initialization
    ScalarVertexProperty varea_0    = mesh->vertex_property<Scalar>("v:area_0",1);
    Vector3VertexProperty points_0  = mesh->vertex_property<Vector3>("v:point_0");
    Vector3VertexProperty points    = mesh->vertex_property<Vector3>("v:point");
    ScalarVertexProperty omega_H    = mesh->vertex_property<Scalar>("v:omega_H",omega_H_0);
    ScalarVertexProperty omega_L    = mesh->vertex_property<Scalar>("v:omega_L",omega_L_0);
    BoolVertexProperty   vissplit   = mesh->vertex_property<bool>("v:issplit",false);
    BoolVertexProperty   visfixed   = mesh->vertex_property<bool>("v:isfixed",false);
    Vector3VertexProperty poles     = mesh->vertex_property<Vector3>("v:poles");
    ScalarVertexProperty omega_P = mesh->vertex_property<Scalar>("v:omega_P",0);
#ifdef USE_POLE_ANGLE   
    ScalarVertexProperty pangle     = mesh->vertex_property<Scalar>("v:pangle");
#endif
    
    /// Init/retrieve properties
    static MeanHelper meanArea;
    if(firststep){
        /// Retrieves poles from the associated mesh
        typedef Surface_mesh::Vertex_property< QList<Vector3> > VSetVertexProperty;
        VSetVertexProperty pset = mesh->vertex_property< QList<Vector3> >("v:pset");
        SurfaceMeshModel polemesh;
        QString basename = QFileInfo(mesh->path).baseName();
        QString currpath = QFileInfo(mesh->path).dir().dirName();
        QString polesfullpath = currpath+"/"+basename+"_poles.off";
        if(!QFileInfo(polesfullpath).exists()) throw StarlabException("Cannot find poles file at: %s",qPrintable(polesfullpath) );
        bool success = attempt_read_as_medial_mesh(polesfullpath,&polemesh);
        if(!success) throw StarlabException("Failed to open pole file");
        Vector3VertexProperty poles_in = polemesh.get_vertex_property<Vector3>("v:point");
        Q_ASSERT(poles_in);
#ifdef USE_POLE_ANGLE
        ScalarVertexProperty angle_in = polemesh.get_vertex_property<Scalar>("v:angle");
        Q_ASSERT(angle_in);
#endif
        
        qDebug() << "Cache v:area_0";
        varea_0 = h.computeVertexVoronoiArea("v:area_0");
        foreach(Vertex v, mesh->vertices()){
            meanArea.push(varea_0[v]);
            points_0[v] = points[v];

            /// Save pole in currene mesh
            poles[v] = poles_in[v];
#ifdef USE_POLE_ANGLE
            pangle[v] = angle_in[v];
#endif            
            /// The initial pole set is trivial
            pset[v].push_back(poles[v]);
        }
    }
       
#if 0
    /// Draw the initial cloud
    foreach(Vertex v, mesh->vertices()){
        // if(vissplit[v]) continue;
        drawArea->drawPoint(points_0[v],1);
    }
#endif
    
    /// ----------------------------------------------------------------------------- /// 
    /// 
    ///                               Solver section
    /// 
    /// ----------------------------------------------------------------------------- /// 
    if(true){
        if(firststep){
            foreach(Vertex v, mesh->vertices()){
                omega_L[v] = omega_L_0;
                omega_H[v] = omega_H_0;
                omega_P[v] = omega_P_0;
            }
        }
                    
        /// Update laplacian
        h.createVertexIndexes();
        h.computeMeanValueHalfEdgeWeights(zero_TH);
        h.createLaplacianMatrix();

        /// Set constraints and solve
        h.setConstraints(omega_H,omega_L,omega_P,poles);
        try{
            h.solve();
            h.extractSolution(VPOINT);
        } catch(StarlabException e){}

        /// Update constraints
        {
            ScalarVertexProperty varea = h.computeVertexVoronoiArea(VAREA);
            foreach(Vertex v, mesh->vertices()){
                /// Leave fixed points really alone
                if(visfixed[v]){
                    omega_L[v] = 0;
                    omega_H[v] = 1.0/zero_TH;
                    omega_P[v] = 0;
                    continue;
                }
                
                omega_L[v] = omega_L_0; // *pow(varea[v]/meanArea, .15);
                omega_H[v] = omega_H_0; // *pow(varea[v]/meanArea,-alpha);
#ifdef USE_POLE_ANGLE
                omega_P[v] = omega_P_0 * angle_weight( pangle[v] );
#else
                omega_P[v] = omega_P_0;
#endif
                /// Ficticious vertices are simply relaxed
                if(vissplit[v]){
                    omega_L[v] = omega_L_0;
                    omega_H[v] = omega_H_0;
                    omega_P[v] = 0;
                }
            }
        }
    }    

    /// ----------------------------------------------------------------------------- /// 
    /// 
    ///                         Topology management section
    /// 
    /// ----------------------------------------------------------------------------- /// 
    // qDebug() << TopologyJanitor(mesh).cleanup(1e-10,edgelength_TH,110);
    // qDebug() << TopologyJanitor_MergePoleSet(mesh).cleanup(1e-10,edgelength_TH,110);
    qDebug() << TopologyJanitor_ClosestPole(mesh).cleanup(zero_TH,edgelength_TH,110);
    
    
    /// ----------------------------------------------------------------------------- /// 
    /// 
    ///                                 VERTEX FIXER
    /// 
    /// ----------------------------------------------------------------------------- /// 
    Scalar elength_fixed = edgelength_TH/10.0;
    foreach(Vertex v, mesh->vertices()){
        if( visfixed[v] ){
            drawArea->drawPoint(points[v],3,Qt::red);        
            continue;
        }
        bool willbefixed = false;
        Counter badcounter=0;        
        foreach(Halfedge h, mesh->onering_hedges(v)){
            Scalar elength = mesh->edge_length(mesh->edge(h));
            if(elength<elength_fixed && !mesh->is_collapse_ok(h))
                badcounter++;
        }        
        willbefixed = (badcounter>=2);
        visfixed[v] = willbefixed;
        if(willbefixed) drawArea->drawPoint(points[v],3,Qt::red);        
    }
    /// ----------------------------------------------------------------------------- /// 
    /// 
    ///                                 STATISTICS
    /// 
    /// ----------------------------------------------------------------------------- /// 
    {
#ifdef USE_POLE_ANGLE
        /// Map pole attraction constraints to colors
        if(false){
            ScalarVertexProperty vquality = mesh->vertex_property<Scalar>(VQUALITY);
            foreach(Vertex v, mesh->vertices()){
                if(vissplit[v]) 
                    vquality[v] = 0;
                else            
                    vquality[v] = omega_P[v];
            }
            ColorizeHelper(mesh).vscalar_to_vcolor(VQUALITY);
        }
#endif
        
        /// Show corresponding attracting poles
        if(false){
            /// Keep the set of poles associated with the point
            typedef QList<Vector3> PoleList;
            typedef Surface_mesh::Vertex_property<PoleList> VSetVertexProperty;
            VSetVertexProperty pset = mesh->vertex_property<PoleList>("v:pset");
            foreach(Vertex v, mesh->vertices())
                foreach( Vector3 pole, pset[v] )
                    drawArea->drawSegment(points[v], pole,1,Qt::gray);
        }
        /// Show ACTIVE attracting poles
        if(false){
            foreach(Vertex v, mesh->vertices())
                if(!vissplit[v]){
                    Vector3 pole  = poles[v];
                    Vector3 point = points[v];
                    drawArea->drawSegment(pole, point,3,Qt::green);  
                }
        }
        /// Mark converged points
        if(false){
            ScalarVertexProperty varea = h.computeVertexVoronoiArea(VAREA);
            foreach(Vertex v, mesh->vertices()){
                if(varea[v]<1e-6)
                    drawArea->drawPoint(points[v],3,Qt::magenta);
            }
        }
                
        /// Show constraints stats
        if(false){
            qDebug() << "===== STATISTICS =====";
            qDebug() << ScalarStatisticsHelper(mesh).statistics("v:omega_H");
            qDebug() << ScalarStatisticsHelper(mesh).statistics("v:omega_L");
            qDebug() << ScalarStatisticsHelper(mesh).statistics("h:weight");
            qDebug() << ScalarStatisticsHelper(mesh).statistics("v:omega_P");
        }
        
        /// Colorize splits
        if(false){
            ScalarVertexProperty vquality = mesh->vertex_property<Scalar>(VQUALITY,0.0);
            foreach(Vertex v, mesh->vertices())
                vquality[v] = ((vissplit[v]) ? 1.0 : 0.0);
            ColorizeHelper(mesh).vscalar_to_vcolor(VQUALITY);
            qDebug() << ScalarStatisticsHelper(mesh).statistics(VQUALITY);
        }
        
        /// Debug edge lengths
        if(false){
            SurfaceMeshHelper(mesh).computeEdgeLengths(ELENGTH);
            qDebug() << ScalarStatisticsHelper(mesh).statistics(ELENGTH);
        }
        
        /// Map contraction constraints to colors
        if(false){
            ScalarVertexProperty vquality = mesh->vertex_property<Scalar>(VQUALITY);
            foreach(Vertex v, mesh->vertices())
                vquality[v] = (omega_L[v]);
            ColorizeHelper(mesh).vscalar_to_vcolor(VQUALITY);
        }
        
        /// Display # vertices
        if(false)
            qDebug() << "Current #vertices" << mesh->n_vertices();
    }
    
    /// Manage garbage
    /// @note IMPORTANT: code above might not be happy!!!!!
    if(false)
        mesh->garbage_collection();
    
    /// Change name and path of the file
    if(firststep){
        QFileInfo fi(mesh->path);
        QString basename = fi.baseName();
        QString currpath = fi.dir().path();
        mesh->path = currpath+"/"+basename+"_ckel.off";
        qDebug() << "Path changed!!"; 
    }
}
