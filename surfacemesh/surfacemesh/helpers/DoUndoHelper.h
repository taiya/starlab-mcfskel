#pragma once
#include "SurfaceMeshHelper.h"
#include <QStack>

class DoUndoHelper : public SurfaceMeshHelper{
private:
    QStack<Vector3VertexProperty> history;

public:
    DoUndoHelper(SurfaceMeshModel* mesh) : SurfaceMeshHelper(mesh){}
    void push(){
        QString name = QString::fromStdString(VPOINT);
        name += "_" + history.size();
        Vector3VertexProperty p = mesh->add_vertex_property<Vector3>(qPrintable(name));
        foreach(Vertex v, mesh->vertices())
            p[v] = points[v];
        history.push(p);
    }
    
    void pop(){
        Vector3VertexProperty p = history.pop();
        foreach(Vertex v, mesh->vertices())
            points[v] = p[v];            
    }
};
