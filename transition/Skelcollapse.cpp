#include "Skelcollapse.h"
#include "TopologyJanitor.h"
#include "TopologyJanitor_ClosestPole.h"

#ifdef MATLAB
    #include "MatlabContractionHelper.h"
#endif
#ifdef CHOLMOD
    #include "EigenContractionHelper.h"
#endif

void Skelcollapse::algorithm_iteration(){  
    contractGeometry();
    updateConstraints();
    updateTopology();
    detectDegeneracies();
}

void Skelcollapse::contractGeometry(){
#ifdef MATLAB
    /// @todo can we move the matlab helper as an inner class and make THAT static?
    static MatlabContractionHelper mch(mesh);
    mch.evolve(omega_H,omega_L,omega_P,poles,zero_TH);   
#endif
    
#ifdef CHOLMOD
    EigenContractionHelper ech(mesh);
    ech.evolve(omega_H,omega_L,omega_P,poles,zero_TH);
#endif
}
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

Q_EXPORT_PLUGIN(Skelcollapse)
