#pragma once
#include "SurfaceMeshHelper.h"
#include <QStack>

class DoUndoHelper : public SurfaceMeshHelper{
private:
    QStack<Vector3fVertexProperty> history;

public:
    DoUndoHelper(SurfaceMeshModel* mesh) : SurfaceMeshHelper(mesh){}
    void push(){
        char* property = VPOINT;
        QString name(property);
        name += "_" + history.size();
        Vector3fVertexProperty p = mesh->add_vertex_property<Vector3f>(name);
        foreach(Vertex v, mesh->vertices())
            p[v] = points[v];
        history.push(p);
    }
    
    void pop(){
        Vector3fVertexProperty p = history.pop();
        foreach(Vertex v, mesh->vertices())
            points[v] = p[v];            
    }
};
