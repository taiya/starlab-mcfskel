#pragma once
#include "SurfaceMeshHelper.h"

namespace SurfaceMesh{

class NormalsHelper : public virtual SurfaceMeshHelper{
public:
    NormalsHelper(SurfaceMeshModel* mesh) : SurfaceMeshHelper(mesh){}

public:
    /// @todo remove dependency from obsolete Surface_mesh::compute_face_normal
    Vector3FaceProperty compute_face_normals(std::string property=FNORMAL){
        Vector3FaceProperty fnormal = mesh->face_property<Vector3>(property);
        foreach(Face f, mesh->faces())
            fnormal[f] = mesh->compute_face_normal(f);
        return fnormal;
    }

    /// @todo remove dependency from obsolete Surface_mesh::compute_vertex_normal
    Vector3VertexProperty compute_vertex_normals(std::string property=VNORMAL){
        Vector3VertexProperty vnormal = mesh->vertex_property<Vector3>(property);
        foreach(Vertex v, mesh->vertices())
            vnormal[v] = mesh->compute_vertex_normal(v);     
        return vnormal;
    }
};

} /// namespace
