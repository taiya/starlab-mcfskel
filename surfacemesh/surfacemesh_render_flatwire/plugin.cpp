#include <qgl.h>
#include "plugin.h"
#include "surface_mesh/gl_wrappers.h"
#include "StarlabDrawArea.h"

using namespace SurfaceMesh;

class FlatwireRenderer : public SurfaceMeshRenderer{
    /// index array for buffered OpenGL rendering
    std::vector<unsigned int> edges;
    std::vector<unsigned int> triangles; 
    
    void init(){
        mesh()->update_face_normals();
        
        /// For wireframe
        edges.clear();
        edges.reserve(mesh()->n_edges());
        Surface_mesh::Edge_iterator eit, eend=mesh()->edges_end();
        for (eit=mesh()->edges_begin(); eit!=eend; ++eit){
            edges.push_back(mesh()->vertex(eit, 0).idx());
            edges.push_back(mesh()->vertex(eit, 1).idx());
        }
        
        /// For flat shading
        triangles.clear();
        Surface_mesh::Face_iterator fit, fend=mesh()->faces_end();
        Surface_mesh::Vertex_around_face_circulator fvit, fvend;
        Surface_mesh::Vertex v0, v1, v2;
        for (fit=mesh()->faces_begin(); fit!=fend; ++fit){
            fvit = fvend = mesh()->vertices(fit);
            v0 = fvit;
            v2 = ++fvit;
            do 
            {
                v1 = v2;
                v2 = fvit;
                triangles.push_back(v0.idx());
                
                triangles.push_back(v1.idx());
                triangles.push_back(v2.idx());
            } 
            while (++fvit != fvend);
        }
    }
    
    void render(){
        glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT | GL_LIGHTING_BIT );
            glEnable(GL_POLYGON_OFFSET_FILL);
            glPolygonOffset(1.0, 1);
            
            render_flat(); /// DrawFill<nm,cm,tm>(); /// < in meshlab
            
            glDisable(GL_POLYGON_OFFSET_FILL);
            glEnable(GL_COLOR_MATERIAL);
            glColorMaterial(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE);
            
            render_wire(); /// DrawWire<nm,CMNone>(); /// < in meshlab
        glPopAttrib();
    }
    
    void render_flat(){
        /// Set the global color
        glColor4d(mesh()->color.redF(), mesh()->color.greenF(), mesh()->color.blueF(), mesh()->color.alphaF());
    
        /// Vertex coordinates
        Surface_mesh::Vertex_property<Point> points = mesh()->vertex_property<Point>("v:point");
        
        /// Per-face colors?
        Surface_mesh::Face_property<Color> fcolor = mesh()->get_face_property<Color>("f:color");
        bool has_face_color = mesh()->has_face_property<Color>("f:color");
        Surface_mesh::Face_property<Point> fnormals = mesh()->get_face_property<Point>("f:normal");
        
        /// @todo convert to "foreach"
        Surface_mesh::Face_iterator fit, fend=mesh()->faces_end();
        Surface_mesh::Vertex_around_face_circulator fvit, fvend;
        for (fit=mesh()->faces_begin(); fit!=fend; ++fit){
            // this will be slow, but we don't want to distinguish between
            // triangles, quads, and general polygons
            // immediate mode rendering is slow anyway...
            glBegin(GL_POLYGON);
                if(has_face_color) 
                    gl::glColor(fcolor[fit]);
                gl::glNormal(fnormals[fit]);
                fvit = fvend = mesh()->vertices(fit);
                do{ gl::glVertex(points[fvit]);
                } while (++fvit != fvend);
            glEnd();
        }
    }
    
    void render_wire(){
        glLineWidth(1.0);
        glColor3f(.3f,.3f,.3f);
        Surface_mesh::Vertex_property<Point> points = mesh()->vertex_property<Point>("v:point");
        gl::glVertexPointer(points.data());
        glEnableClientState(GL_VERTEX_ARRAY);
        /// Render
        glDisable(GL_LIGHTING);
        glColor3f(0.0, 0.0, 0.0);
        glDepthRange(0.0, 1.0);
        glDepthFunc(GL_LEQUAL);
        if(edges.size()) glDrawElements(GL_LINES, (GLsizei)edges.size(), GL_UNSIGNED_INT, &edges[0]);
        glDepthFunc(GL_LESS);
    }
};

Renderer* plugin::instance(){ return new FlatwireRenderer(); }

Q_EXPORT_PLUGIN(plugin)
