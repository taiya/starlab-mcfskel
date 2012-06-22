#include "Skelcollapse.h"

#include <QList>
#include <QDir>

#include "surfacemesh_io_helpers.h"
#include "LegacyLaplacianHelper.h"
#include "PoleAttractorHelper.h"
#include "TopologyJanitor.h"
#include "TopologyJanitor_ClosestPole.h"

void Skelcollapse::applyFilter(Document* document, RichParameterSet* pars, StarlabDrawArea* drawArea){
    /// Filter initialization
    SurfaceMeshModel* mesh = qobject_cast<SurfaceMeshModel*>( document->selectedModel() );
    drawArea->deleteAllRenderObjects();
    
    Scalar omega_L_0 = pars->getFloat("omega_L_0"); 
    Scalar omega_H_0 = pars->getFloat("omega_H_0");
    Scalar omega_P_0 = pars->getFloat("omega_P_0");
    Scalar edgelength_TH = pars->getFloat("edgelength_TH");
    Scalar zero_TH = pars->getFloat("zero_TH");
        
    /// Compute initialization
    Vector3VertexProperty points    = mesh->vertex_property<Vector3>("v:point");
    ScalarVertexProperty omega_H    = mesh->vertex_property<Scalar>("v:omega_H",omega_H_0);
    ScalarVertexProperty omega_L    = mesh->vertex_property<Scalar>("v:omega_L",omega_L_0);
    ScalarVertexProperty omega_P = mesh->vertex_property<Scalar>("v:omega_P",0);
    BoolVertexProperty   vissplit   = mesh->vertex_property<bool>("v:issplit",false);
    BoolVertexProperty   visfixed   = mesh->vertex_property<bool>("v:isfixed",false);
    Vector3VertexProperty poles     = mesh->vertex_property<Vector3>("v:pole");
    
    /// LEGACY!!!!!!!!!!
    typedef Surface_mesh::Vertex_property< QList<Vector3> > VSetVertexProperty;
    VSetVertexProperty pset = mesh->vertex_property< QList<Vector3> >("v:pset");
    bool firststep = !mesh->property("ContractionInitialized").isValid();
    if(firststep){
        mesh->setProperty("ContractionInitialized",true);
        /// Every vertex initially corresponds to itself
        // foreach(Vertex v, mesh->vertices())
        //  corrs[v].push_back(v);

        /// LEGACY!!!!!!!!!!
        foreach(Vertex v, mesh->vertices()){
            pset[v].push_back(poles[v]);            
        }
    }
       
    /// ----------------------------------------------------------------------------- /// 
    /// 
    ///                               Solver section
    /// 
    /// ----------------------------------------------------------------------------- /// 
    if(true){
        static PoleAttractorHelper h(mesh);
        h.eval("lastwarn('');");       
        
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
                
                omega_L[v] = omega_L_0;
                omega_H[v] = omega_H_0;
                omega_P[v] = omega_P_0;

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

    /// Change name and path of the file
    if(firststep){
        QFileInfo fi(mesh->path);
        QString basename = fi.baseName();
        QString currpath = fi.dir().path();
        mesh->path = currpath+"/"+basename+"_ckel.off";
        qDebug() << "Path changed!!"; 
    }
}

Q_EXPORT_PLUGIN(Skelcollapse)
