/// DO NOT INCLUDE EXPLICITLY, THIS IS INCLUDED BY SurfaceMeshModel.h

#pragma once
#include <iterator>
namespace SurfaceMesh{
    class VertexCoordinatesIterator : public std::iterator<std::forward_iterator_tag,Vector3>{
    private:
        Surface_mesh::Vertex_iterator it;
        typedef VertexCoordinatesIterator Iterator;
        Vector3VertexProperty points;
        Model* mesh;
    public:
        VertexCoordinatesIterator(Model* mesh){
            points = mesh->vertex_coordinates();
            it = mesh->vertices_begin();
            this->mesh = mesh;
        }
    public:
        bool operator!=(Iterator it){ return this->it != it.it; }
        /// prefix++
        Iterator&  operator++(){ ++it; return *this; } // ++myInstance.
        /// postfix++ (uses prefix)
        Iterator operator++ (int /*val*/){ return operator++(); }
        value_type* operator->(){ return &( points[ Vertex(it) ] ); }
        value_type  operator*(){ return points[ Vertex(it) ]; }
        VertexCoordinatesIterator begin(){  this->it=mesh->vertices_begin(); return *this; }
        VertexCoordinatesIterator end(){ this->it=mesh->vertices_end();   return *this; }
    };
} // namespace
