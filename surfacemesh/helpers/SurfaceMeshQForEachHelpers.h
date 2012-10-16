#pragma once

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
