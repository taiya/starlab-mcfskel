#pragma once
#include "CurveskelModel.h"

namespace CurveskelTypes
{
    class CurveskelForEachEdgeHelper{
    private:
        typedef CurveskelModel::Edge Edge;
        CurveskelModel* m;
    public:
        CurveskelForEachEdgeHelper(CurveskelModel* _m):m(_m){}
        enum etype{BEGIN,END};
        class const_iterator{
        private:
            CurveskelModel* m;
            CurveskelModel::Edge_iterator eit;
        public:
            const_iterator(CurveskelModel* _m, etype e) : m(_m){
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

    class CurveskelForEachVertexHelper{
    private:
        typedef CurveskelModel::Vertex Vertex;
        CurveskelModel* m;
    public:
        CurveskelForEachVertexHelper(CurveskelModel* _m):m(_m){}
        enum etype{BEGIN,END};
        class const_iterator{
        private:
            CurveskelModel* m;
            CurveskelModel::Vertex_iterator vit;
        public:
            const_iterator(CurveskelModel* _m, etype e) : m(_m){
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
}
