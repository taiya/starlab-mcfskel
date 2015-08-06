#include <qgl.h>
#include "plugin.h"
#include "surface_mesh/gl_wrappers.h"

using namespace SurfaceMesh;

class WireframeRenderer : public SurfaceMeshRenderer{
#ifdef TODO_WIREFRAME_VBUFFER
    /// index array for buffered OpenGL rendering
    std::vector<unsigned int>  edges;
#endif 
    
    void init(){
        // qDebug() << "surfacemesh_render_wireframe::init";
        // mesh()->update_face_normals();
    
    #ifdef TODO_WIREFRAME_VBUFFER
        edges.clear();
        edges.reserve(mesh()->n_edges());
        Surface_mesh::Edge_iterator eit, eend=mesh()->edges_end();
        for (eit=mesh()->edges_begin(); eit!=eend; ++eit){
            edges.push_back(mesh()->vertex(eit, 0).idx());
            edges.push_back(mesh()->vertex(eit, 1).idx());
        }
    #endif
    }    

    void render(){
        glDisable(GL_LIGHTING);
    
    /// @todo Much faster but does not support color...YET
    #ifdef TODO_WIREFRAME_VBUFFER
        /// Setup Vertex Array
        Surface_mesh::Vertex_property<Point> points = mesh()->vertex_property<Point>("v:point");
        gl::glVertexPointer(points.data());
        glEnableClientState(GL_VERTEX_ARRAY);
        /// Render
        glColor3f(0.0, 0.0, 0.0);
        glDepthRange(0.0, 1.0);
        glDepthFunc(GL_LEQUAL);
        if(edges.size()) glDrawElements(GL_LINES, (GLsizei)edges.size(), GL_UNSIGNED_INT, &edges[0]);
        glDepthFunc(GL_LESS);
    #else
        Surface_mesh::Vertex_property<Point> points = mesh()->vertex_property<Point>("v:point");
        Surface_mesh::Edge_property<Color> ecolor = mesh()->get_edge_property<Color>("e:color");
        bool has_edge_color = mesh()->has_edge_property<Color>("e:color");
        glBegin(GL_LINES);
        Surface_mesh::Edge_iterator eit, eend=mesh()->edges_end();
        for (eit=mesh()->edges_begin(); eit!=eend; ++eit){
            Surface_mesh::Edge e = eit;
            Surface_mesh::Vertex v0 = mesh()->vertex(eit,0);
            Surface_mesh::Vertex v1 = mesh()->vertex(eit,1);
    
            if(has_edge_color)
                gl::glColor(ecolor[eit]); ///< Apply color
            else
                gl::glColor(Color(0,0,0));
    
            gl::glVertex(points[v0]);
            gl::glVertex(points[v1]);
        }
        glEnd();
    #endif
    
        glEnable(GL_LIGHTING);
    }
    
};

Renderer* plugin::instance(){ return new WireframeRenderer(); }

Q_EXPORT_PLUGIN(plugin)
