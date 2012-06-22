#include "mcfskel.h"

/// QT-stuff
#include <QList>
#include <QDir>
#include <QFileDialog>

/// Dictates the motion 
#include "ContractionHelper.h"
#include "TopologyJanitor.h"
#include "PoleAttractorHelper.h"

void mcfskel::applyFilter(SurfaceMeshModel* mesh, RichParameterSet *pars, StarlabDrawArea *drawArea){
    // if(firststep) mesh->renderer()->setRenderMode("Smooth");

    /// Retrieve parameters
    Scalar omega_L_0 = pars->getFloat("omega_L_0");
    Scalar omega_H_0 = pars->getFloat("omega_H_0");
    Scalar omega_P_0 = pars->getFloat("omega_P_0");
    Scalar edgelength_TH = pars->getFloat("edgelength_TH");
    Scalar zero_TH = pars->getFloat("zero_TH");
    bool medialContraction = pars->getBool("medialContraction");
    bool performContraction = pars->getBool("performContraction");
    
    /// This is just logical, but we can get better performance
    if(medialContraction==false)
        omega_P_0 = 0.0;
    
    /// Compute initialization 
    /// @todo some properties not needed? will it drastically improve performances?
    Vector3VertexProperty    points    = mesh->vertex_property<Vector3>("v:point");
    ScalarVertexProperty     omega_L   = mesh->vertex_property<Scalar>("v:omega_L",omega_L_0);
    ScalarVertexProperty     omega_H   = mesh->vertex_property<Scalar>("v:omega_H",omega_H_0);
    ScalarVertexProperty     omega_P   = mesh->vertex_property<Scalar>("v:omega_P",omega_P_0);
    BoolVertexProperty       vissplit  = mesh->vertex_property<bool>("v:issplit",false);
    BoolVertexProperty       visfixed  = mesh->vertex_property<bool>("v:isfixed",false);
    Vector3VertexProperty    poles     = mesh->vertex_property<Vector3>("v:pole",Vector3(0));
    VertexListVertexProperty corrs     = mesh->vertex_property<VertexList>("v:corrs");
    ScalarHalfedgeProperty   hweight   = mesh->halfedge_property<Scalar>("h:weight",0.0);
    
    if( !mesh->property("ContractionInitialized").isValid() ){
        mesh->setProperty("ContractionInitialized",true);
        /// Every vertex initially corresponds to itself
        foreach(Vertex v, mesh->vertices())
            corrs[v].push_back(v);
    }

#if 0
    drawArea->deleteAllRenderObjects();
    foreach(Vertex v, mesh->vertices())
        drawArea->drawPoint(points[v]);
#endif
    
    /// ----------------------------------------------------------------------------- ///
    ///
    ///                               Solver section
    ///
    /// ----------------------------------------------------------------------------- ///
    if(performContraction){
#if false
        ContractionHelper h(mesh);
        
        /// Solve the system
        // computeEdgeWeights(zero_TH);
        // hweight = h.cotangentHelper.computeEdgeWeights("h:weight");
        hweight = h.meanvalueHelper.computeEdgeWeights("h:weight",zero_TH);
        h.updateVertexIndexes();
        h.createLHS(hweight,omega_L,omega_H);
        h.createRHS(omega_H,points);
        h.solveByFactorization(VPOINT);
        
        /// Update the constraints
        foreach(Vertex v, mesh->vertices()){
            omega_L[v] = visfixed[v] ? 0.0         : omega_L_0;
            omega_H[v] = visfixed[v] ? 1.0/zero_TH : omega_H_0;
            omega_P[v] = vissplit[v] ? 0.0         : omega_P_0;
        }
#else
        static PoleAttractorHelper h(mesh);
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
            h.computeEdgeWeights("e:weight",zero_TH);
            h.createLaplacianMatrix();
    
            /// Set constraints and solve
            h.setConstraints(omega_H,omega_L,omega_P,poles);
            try{
                h.solve();
                h.extractSolution(VPOINT);
            } catch(StarlabException e){}
    
            /// Update constraints
            {
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
        
        
#endif
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
    ///                         Check for converged vertices
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
}

Q_EXPORT_PLUGIN(mcfskel)
