#include <qgl.h>
#include "surfacemesh_render_smooth.h"
#include "surface_mesh/gl_wrappers.h"

void surfacemesh_render_smooth::init(){
    // qDebug() << "surfacemesh_render_flat::init";
    mesh()->update_face_normals();
    mesh()->update_vertex_normals();

    /// Initialize triangle buffer
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

void surfacemesh_render_smooth::render(){
    Surface_mesh::Vertex_property<Point>  points = mesh()->vertex_property<Point>("v:point");    
	Surface_mesh::Vertex_property<Point>  vnormals = mesh()->vertex_property<Point>("v:normal");

	// Deal with color
	bool has_vertex_color = mesh()->has_vertex_property<Color>("v:color");
	Surface_mesh::Vertex_property<Color>  vcolor;
	if (has_vertex_color) vcolor = mesh()->get_vertex_property<Color>("v:color");

	// setup vertex arrays    
	gl::glVertexPointer(points.data());
	gl::glNormalPointer(vnormals.data());
	if(has_vertex_color) gl::glColorPointer(vcolor.data());
	else glColor4dv(Vec4d(0.75,0.75,0.75,1.0));

	glEnable(GL_LIGHTING);
	glShadeModel(GL_SMOOTH);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	if(has_vertex_color) glEnableClientState(GL_COLOR_ARRAY);
	if(triangles.size()) glDrawElements(GL_TRIANGLES, (GLsizei)triangles.size(), GL_UNSIGNED_INT, &triangles[0]);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
}

Q_EXPORT_PLUGIN(surfacemesh_render_smooth)
