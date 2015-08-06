#pragma once
#include "SurfaceMeshModel.h"
#include "StarlabException.h"
#include "SurfaceMeshHelper.h"

class LaplacianHelper : public virtual SurfaceMeshHelper{
protected:
    Vector3VertexProperty laplace;
  
public:
    LaplacianHelper(SurfaceMeshModel* mesh) : SurfaceMeshHelper(mesh){
        laplace = mesh->vertex_property<Point>("v:laplace",Vector3(0,0,0));        
    }
    virtual void applyLaplacianIteration(float damping=0.5f){
        foreach(Vertex v, mesh->vertices())
            points[v] += damping * laplace[v];
    }
    Vector3VertexProperty computeLaplacianVectors(const std::string property="h:weight"){
        laplace = mesh->vertex_property<Vector3>(property);
        foreach(Vertex v, mesh->vertices()){
            laplace[v] = Vector3(0,0,0);
            Vector3 p = points[v];
            /// Up to this point it is the centroid
            foreach(Halfedge hi, mesh->onering_hedges(v)){
                Vertex vi = mesh->to_vertex(hi);
                Vector3 pi = points[vi];
                laplace[v] += pi;
            }
            laplace[v] /= (Scalar) mesh->valence(v);
            /// Now it is the comb laplacian
            laplace[v] = laplace[v]-p;
        }
        return laplace;
    }
};
