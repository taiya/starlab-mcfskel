#pragma once
#include "ColorMap.h"
#include "SurfaceMeshModel.h"
#include "StarlabException.h"
#include "SurfaceMeshHelper.h"
#include "float.h"
#include <cmath>

class ColorizeHelper : SurfaceMeshHelper{
    ColorMap* cmap;
    
public:
    ColorizeHelper(SurfaceMeshModel* mesh) : SurfaceMeshHelper(mesh){
        cmap = &signedColorMap;
    }
    /// Allows for ColorizeHelper(mesh,SignedColorMap()).foo()
    ColorizeHelper(SurfaceMeshModel* mesh, ColorMap& cmap) : SurfaceMeshHelper(mesh){
        this->cmap = &cmap;
    }
    
public:
    void vscalarratio_to_vcolor(char* p1name, char* p2name){
        ScalarVertexProperty p1 = mesh->get_vertex_property<Scalar>(p1name);
        ScalarVertexProperty p2 = mesh->get_vertex_property<Scalar>(p2name);
        ScalarVertexProperty temp = mesh->add_vertex_property<Scalar>("v:tempratio");
        
        /// Compute ratio
        foreach(Vertex v, mesh->vertices()){
            temp[v] = p1[v]/p2[v];
            qDebug() << "ratio" << temp[v] << p1[v] << p2[v];
        }
        
        /// Colorize it
        vscalar_to_vcolor("v:tempratio");
        mesh->remove_vertex_property(temp);
    }
    
    void escalar_to_ecolor( char* property="e:quality"){
        Surface_mesh::Edge_property<Scalar> escalar = mesh->get_edge_property<Scalar>(property);
        Surface_mesh::Edge_property<Color>  ecolor   = mesh->edge_property<Color>("e:color",Color(0,0,0));
        
        /// Compute bounds
        float min=FLT_MAX,max=-FLT_MAX;
        foreach(Edge eit,mesh->edges()){
            min = (escalar[eit]<min) ? escalar[eit]:min;
            max = (escalar[eit]>max) ? escalar[eit]:max;
        }      
        
        cmap->init(min,max);

        /// Apply colormap
        foreach(Edge eit, mesh->edges())
            ecolor[eit] = cmap->color(escalar[eit]);
    }
    
    void vscalar_to_vcolor(const std::string property="v:quality"){
        if(!mesh->has_vertex_property<Scalar>(property)) 
            throw StarlabException("Mesh doesn't have %s how do you expect me to colorize it?", property.c_str());
        Surface_mesh::Vertex_property<Scalar> vscalar = mesh->get_vertex_property<Scalar>(property);
        Surface_mesh::Vertex_property<Color>  vcolor   = mesh->vertex_property<Color>("v:color",Color(0,0,0));
        if(!vscalar) throw StarlabException("Requested property missing");
        
        /// Compute quality for vertex collapse    
        float min=FLT_MAX,max=-FLT_MAX;
        foreach(Vertex v,mesh->vertices()){
            min = (vscalar[v]<min) ? vscalar[v]:min;
            max = (vscalar[v]>max) ? vscalar[v]:max;
        }

        cmap->init(min,max);        
              
        /// Apply colormap
        foreach(Vertex v, mesh->vertices()){
            Scalar val = vscalar[v];
            if(std::isnan(val)) qDebug() << "Warning: colorizing 'nan' value";
            if(std::isinf(val)) qDebug() << "Warning: colorizing 'inf' value";
            vcolor[v] = cmap->color(vscalar[v]);        
        }
    }
};
