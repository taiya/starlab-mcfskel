#pragma once
#include "SurfaceMeshHelper.h"

class SurfaceAreaHelper : public virtual SurfaceMeshHelper{
protected:
    ScalarFaceProperty farea; /// NULL                
    ScalarFaceProperty varea; /// NULL                

public:
    SurfaceAreaHelper(SurfaceMeshModel* mesh) : SurfaceMeshHelper(mesh){}
    
    ScalarFaceProperty computeFaceAreas(const string property=FAREA){
        farea = mesh->face_property<Scalar>(property);                
        foreach(Face f, mesh->faces()){
            Surface_mesh::Vertex_around_face_circulator vfit = mesh->vertices(f);
            Vertex v0 = vfit;
            Vertex v1 = ++vfit;
            Vertex v2 = ++vfit;
            farea[f] = 0.5 * cross(points[v1]-points[v0], points[v2]-points[v0]).norm();
        }
        return farea;
    }
};
