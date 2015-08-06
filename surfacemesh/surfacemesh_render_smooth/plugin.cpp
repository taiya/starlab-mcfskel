#include "plugin.h"
#include <QtOpenGL>
#include "surface_mesh/gl_wrappers.h"

using namespace SurfaceMesh;

class SmoothRenderer : public SurfaceMeshRenderer{
    ///< index array for buffered OpenGL rendering
    std::vector<unsigned int> triangles; 
    
    void init(){
        // qDebug() << "surfacemesh_render_flat::init";
        mesh()->update_face_normals();
        mesh()->update_vertex_normals();
    
        /// Initialize triangle buffer
        triangles.clear();
        foreach(Face f, mesh()->faces())
            foreach(Vertex v, mesh()->vertices(f))
                triangles.push_back(v.idx());
    }
    
    void render(){
		if(mesh()->n_faces() < 1) return;

        Surface_mesh::Vertex_property<Point>  points = mesh()->vertex_property<Point>(VPOINT);
        Surface_mesh::Vertex_property<Point>  vnormals = mesh()->vertex_property<Point>(VNORMAL);
    
        // Deal with color
        bool has_vertex_color = mesh()->has_vertex_property<Color>(VCOLOR);
        Surface_mesh::Vertex_property<Color>  vcolor;
        if (has_vertex_color) vcolor = mesh()->get_vertex_property<Color>(VCOLOR);
    
        // setup vertex arrays    
        gl::glVertexPointer(points.data());
        gl::glNormalPointer(vnormals.data());
        if(has_vertex_color) 
            gl::glColorPointer(vcolor.data());
    
        glEnable(GL_LIGHTING);
        glShadeModel(GL_SMOOTH);
        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_NORMAL_ARRAY);
        if(has_vertex_color) glEnableClientState(GL_COLOR_ARRAY);
        if(triangles.size()) glDrawElements(GL_TRIANGLES, (GLsizei)triangles.size(), GL_UNSIGNED_INT, &triangles[0]);
        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_NORMAL_ARRAY);
    }
};

Renderer* plugin::instance(){ return new SmoothRenderer(); }

Q_EXPORT_PLUGIN(plugin)
