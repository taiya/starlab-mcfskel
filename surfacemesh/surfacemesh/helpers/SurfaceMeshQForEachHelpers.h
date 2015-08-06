#pragma once
#include "surface_mesh/Surface_mesh.h"

namespace SurfaceMesh{
class SurfaceMeshModel;

/// @see SurfaceMeshModel::halfedges()
class SurfaceMeshForEachHalfedgeHelper{
private:
    typedef SurfaceMeshModel::Halfedge Halfedge;
    SurfaceMeshModel* m;
public:
    SurfaceMeshForEachHalfedgeHelper(SurfaceMeshModel* _m):m(_m){}
    enum etype{BEGIN,END};
    class const_iterator{
    private:
        SurfaceMeshModel* m;
        SurfaceMeshModel::Halfedge_iterator hit;
    public:
        const_iterator(SurfaceMeshModel* _m, etype e) : m(_m){
            if(e==BEGIN) hit=m->halfedges_begin();
            if(e==END) hit=m->halfedges_end();
        }
        const Halfedge operator*(){ return (const Halfedge) hit; }
        bool operator!=( const const_iterator& rhs) const{ return hit!=rhs.hit; }
        const_iterator& operator++(){ ++hit; return *this; }
    };

    const_iterator begin() const{return const_iterator(m,BEGIN);}
    const_iterator end() const{return const_iterator(m,END);}   
};

/// @see SurfaceMeshModel::vertices()
class SurfaceMeshForEachVertexHelper{
private: 
    typedef SurfaceMeshModel::Vertex Vertex;
    SurfaceMeshModel* m;
public:
    SurfaceMeshForEachVertexHelper(SurfaceMeshModel* _m):m(_m){}
    enum etype{BEGIN,END};
    class const_iterator{
    private:
        SurfaceMeshModel* m;
        SurfaceMeshModel::Vertex_iterator vit;
    public:
        const_iterator(SurfaceMeshModel* _m, etype e) : m(_m){
            if(e==BEGIN) vit=m->vertices_begin();
            if(e==END) vit=m->vertices_end();
        }
        const Vertex operator*(){ return (const Vertex) vit; }
        bool operator!=( const const_iterator& rhs) const{ return vit!=rhs.vit; }
        const_iterator& operator++(){ ++vit; return *this; }
    };

    const_iterator begin() const{return const_iterator(m,BEGIN);}
    const_iterator end() const{return const_iterator(m,END);}   
};

/// Performance loss: measure
class SurfaceMeshForEachFaceAtVertex : public Surface_mesh::Face_around_vertex_circulator{
private:
    typedef Surface_mesh::Face_around_vertex_circulator Super;
    typedef SurfaceMeshModel::Vertex Vertex;
    typedef SurfaceMeshModel::Face Face;
public:
    /// @internal used by Qt::foreach
    typedef SurfaceMeshForEachFaceAtVertex const_iterator;
private:
    SurfaceMeshModel* _m;
    Vertex _v;
    bool disbelieve;
public:
    /// This let the traditional Vertex_around_face_circulator pass through
    SurfaceMeshForEachFaceAtVertex(SurfaceMeshModel* m, Vertex v) 
        : Face_around_vertex_circulator(m, v), _m(m), _v(v),disbelieve(false){}
public:
    /// In first iteration begin=end as we are in a loop, this operator prevents this from happening
    bool operator!=(const SurfaceMeshForEachFaceAtVertex& rhs) const{
        bool diffs = Super::operator !=(rhs);
        if(!diffs && disbelieve) return true;
        return diffs;
    }
    /// after first increment I am not at the origin anymore
    Face_around_vertex_circulator& operator++(){
        disbelieve=false; 
        return Super::operator ++();
    }
    /// add dereferencing to "Vertex_around_face_circulator"
    const Face operator*(){ 
        return this->operator Face(); 
    }
public:
    /// @internal used by Qt::foreach
    const_iterator begin() const{return const_iterator(_m,_v,true);}
    /// @internal used by Qt::foreach
    const_iterator end() const{return const_iterator(_m,_v,false);}
private:
    /// @internal used by Qt::foreach
    SurfaceMeshForEachFaceAtVertex(SurfaceMeshModel* m, Vertex v, bool disbelieve)
        : Face_around_vertex_circulator(m, v),_m(m),_v(v),disbelieve(disbelieve){}   
};

/// Performance loss: FOREACH 47ms, TRADITIONAL 39ms
class SurfaceMeshForEachVertexOnFaceHelper : public Surface_mesh::Vertex_around_face_circulator{
private:
    typedef Surface_mesh::Vertex_around_face_circulator Super;
    typedef SurfaceMeshModel::Vertex Vertex;
    typedef SurfaceMeshModel::Face Face;
public:
    /// @internal used by Qt::foreach
    typedef SurfaceMeshForEachVertexOnFaceHelper const_iterator;
private:
    SurfaceMeshModel* _m;
    Face _f;
    bool disbelieve;
public:
    /// This let the traditional Vertex_around_face_circulator pass through
    SurfaceMeshForEachVertexOnFaceHelper(SurfaceMeshModel* m, Face f) 
        : Vertex_around_face_circulator(m, f),_m(m),_f(f),disbelieve(false){}
public:
    /// In first iteration begin=end as we are in a loop, this operator prevents this from happening
    bool operator!=(const SurfaceMeshForEachVertexOnFaceHelper& rhs) const{
        bool diffs = Super::operator !=(rhs);
        if(!diffs && disbelieve) return true;
        return diffs;
    }
    /// after first increment I am not at the origin anymore
    Vertex_around_face_circulator& operator++(){
        disbelieve=false; 
        return Super::operator ++();
    }
    /// add dereferencing to "Vertex_around_face_circulator"
    const Vertex operator*(){ 
        return this->operator Vertex(); 
    }
public:
    /// @internal used by Qt::foreach
    const_iterator begin() const{return const_iterator(_m,_f,true);}
    /// @internal used by Qt::foreach
    const_iterator end() const{return const_iterator(_m,_f,false);}
private:
    /// @internal used by Qt::foreach
    SurfaceMeshForEachVertexOnFaceHelper(SurfaceMeshModel* m, Face f, bool disbelieve)
        : Vertex_around_face_circulator(m, f),_m(m),_f(f),disbelieve(disbelieve){}   
};

/// @see SurfaceMeshModel::oneringEdges()
class SurfaceMeshForEachOneRingEdgesHelper{
private:
    typedef SurfaceMeshModel::Vertex Vertex;
    typedef SurfaceMeshModel::Halfedge Halfedge;
    
    SurfaceMeshModel* m;
    Vertex v;
public:
    SurfaceMeshForEachOneRingEdgesHelper(SurfaceMeshModel* _m,Vertex _v):m(_m),v(_v){}
    enum etype{BEGIN,END};
    class const_iterator{
    private:
        SurfaceMeshModel* m;
        Vertex v;
        bool first; /// do-while like iteration
        SurfaceMeshModel::Halfedge_around_vertex_circulator hit;
    public:
        const_iterator(SurfaceMeshModel* _m, Vertex _v, etype e) : m(_m),v(_v),first(true){
            if(e==BEGIN) hit = _m->halfedges(v);
            if(e==END) hit = _m->halfedges(v);
        }
        const Halfedge operator*(){ 
            return hit; 
        }
        bool operator!=( const const_iterator& rhs) const{ 
            if( m->is_isolated(v) ) return false;
            if( first==true ){ return true; }
            return hit!=rhs.hit; 
        }
        const_iterator& operator++(){ 
            first=false; ++hit; 
            return *this; 
        }
    };
    const_iterator begin() const{return const_iterator(m,v,BEGIN);}
    const_iterator end() const{return const_iterator(m,v,END);}   
};

class SurfaceMeshForEachFaceHelper{
private: 
    typedef SurfaceMeshModel::Face Face;
    SurfaceMeshModel* m;
public:
    SurfaceMeshForEachFaceHelper(SurfaceMeshModel* _m):m(_m){}
    enum etype{BEGIN,END};
    class const_iterator{
    private:
        SurfaceMeshModel* m;
        SurfaceMeshModel::Face_iterator fit;
    public:
        const_iterator(SurfaceMeshModel* _m, etype e) : m(_m){
            if(e==BEGIN) fit=m->faces_begin();
            if(e==END) fit=m->faces_end();
        }
        const Face operator*(){ return (const Face) fit; }
        bool operator!=( const const_iterator& rhs) const{ return fit!=rhs.fit; }
        const_iterator& operator++(){ ++fit; return *this; }
    };

    const_iterator begin() const{return const_iterator(m,BEGIN);}
    const_iterator end() const{return const_iterator(m,END);}
};

/// @see SurfaceMeshModel::edges()
class SurfaceMeshForEachEdgeHelper{
private: 
    typedef SurfaceMeshModel::Edge Edge;
    SurfaceMeshModel* m;
public:
    SurfaceMeshForEachEdgeHelper(SurfaceMeshModel* _m):m(_m){}
    enum etype{BEGIN,END};
    class const_iterator{
    private:
        SurfaceMeshModel* m;
        SurfaceMeshModel::Edge_iterator eit;
    public:
        const_iterator(SurfaceMeshModel* _m, etype e) : m(_m){
            if(e==BEGIN) eit=m->edges_begin();
            if(e==END) eit=m->edges_end();
        }
        const Edge operator*(){ return (const Edge) eit; }
        bool operator!=( const const_iterator& rhs) const{ return eit!=rhs.eit; }
        const_iterator& operator++(){ ++eit; return *this; }
    };

    const_iterator begin() const{return const_iterator(m,BEGIN);}
    const_iterator end() const{return const_iterator(m,END);}   
};

}
