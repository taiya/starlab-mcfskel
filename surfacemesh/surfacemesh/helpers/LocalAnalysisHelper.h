#pragma once
#include "SurfaceMeshHelper.h"
#include "StarlabDrawArea.h"

class LocalAnalysisHelper : public SurfaceMeshHelper{
public:
    LocalAnalysisHelper(SurfaceMeshModel* mesh) : SurfaceMeshHelper(mesh){}
    void markUmbilic(char* property, DrawArea* drawarea){
        Scalar eps = 0; //1e-3;
                
        ScalarVertexProperty p = mesh->vertex_property<Scalar>(property);
        foreach(Vertex v, mesh->vertices()){
            Scalar val = p[v];
            bool ismin = true;
            bool ismax = true;
            foreach(Halfedge h, mesh->onering_hedges(v)){
                Scalar curval = p[ mesh->to_vertex(h) ];
                if(curval<val) ismin=false;
                if(curval+eps>val) ismax=false;
            } 

            // if(ismin) drawarea->drawPoint(points[v],5,Qt::red);
            if(ismax) drawarea->drawPoint(points[v],5,Qt::red); 
        }
        
    }
};
