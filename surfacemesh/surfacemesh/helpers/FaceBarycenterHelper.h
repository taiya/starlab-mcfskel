#pragma once
#include "SurfaceMeshHelper.h"

namespace SurfaceMesh{
    
/// name used by this helper to store information
const std::string FBARYCENTER = "f:barycenter";

/// @brief a helper class to compute face centroids of 
/// Usage: 
/// @see http://en.wikipedia.org/wiki/Centroid
class FaceBarycenterHelper : public SurfaceMeshHelper{
    Vector3FaceProperty fbarycenter;
    Vector3VertexProperty vpoints;
    
public:
    FaceBarycenterHelper(SurfaceMeshModel* mesh) : SurfaceMeshHelper(mesh){
        fbarycenter = mesh->face_property<Vector3>(FBARYCENTER);        
        vpoints = mesh->vertex_property<Vector3>(VPOINT);       
    }    

    /// Allows the usage of: 
    /// Vector3FaceProperty fbary = FaceBarycenterHelper(mesh);
    /// Uses SurfaceMesh::FBARYCENTER to store the property
    operator Vector3FaceProperty(){ return compute(); }

    /// main function
    Vector3FaceProperty compute(){
        foreach(Face fit, mesh->faces())
            fbarycenter[fit] = barycenter(fit);
        return fbarycenter;
    }
    
    /// helper function
    Vector3 barycenter(Face fit){
        int counter = 0;
        Vector3 accumulator(0,0,0);
        foreach(Vertex v, mesh->vertices(fit)){
            accumulator += vpoints[v];
            counter++;
        }
        accumulator = accumulator / counter;
        return accumulator;     
    }
};

} // namespace
