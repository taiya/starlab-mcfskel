#include "Skelcollapse.h"

#include <QList>
#include <QDir>

#include "LegacyLaplacianHelper.h"
#include "PoleAttractorHelper.h"
#include "TopologyJanitor.h"
#include "TopologyJanitor_ClosestPole.h"

void Skelcollapse::updateConstraints(){
    SurfaceMeshHelper h(mesh);
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
void Skelcollapse::contractGeometry(){
    static PoleAttractorHelper h(mesh);
    h.eval("lastwarn('');");       
                       
    /// Update laplacian
    h.createVertexIndexes();
    h.computeMeanValueHalfEdgeWeights(zero_TH);
    h.createLaplacianMatrix();

    /// Set constraints and solve
    h.setConstraints(omega_H,omega_L,omega_P,poles);
    h.solve();
    h.extractSolution(VPOINT);
}
void Skelcollapse::detectDegeneracies(){
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
}
void Skelcollapse::updateTopology(){
    // qDebug() << TopologyJanitor(mesh).cleanup(zero_TH,edgelength_TH,110);
    // qDebug() << TopologyJanitor_MergePoleSet(mesh).cleanup(zero_TH,edgelength_TH,110);
    qDebug() << TopologyJanitor_ClosestPole(mesh).cleanup(zero_TH,edgelength_TH,110);
}

void Skelcollapse::algorithm_iteration(){  
    /// LEGACY!!!!!!!!!!
    typedef Surface_mesh::Vertex_property< QList<Vector3> > VSetVertexProperty;
    VSetVertexProperty pset = mesh->vertex_property< QList<Vector3> >("v:pset");
    if(!isInitialized){
        /// Every vertex initially corresponds to itself
        // foreach(Vertex v, mesh->vertices())
        //  corrs[v].push_back(v);

        /// LEGACY!!!!!!!!!!
        foreach(Vertex v, mesh->vertices()){
            pset[v].push_back(poles[v]);            
        }
    }
       
    contractGeometry();
    updateConstraints();
    updateTopology();
    detectDegeneracies();
}

Q_EXPORT_PLUGIN(Skelcollapse)
