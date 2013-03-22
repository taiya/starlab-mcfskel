#include "curveskel_render_lines.h"
#include "CurveskelQForEach.h"
#include <QtOpenGL>

class CurveSkelLineRenderer : public CurveskelModelRenderer{

    void render(){
        glClear(GL_DEPTH_BUFFER_BIT);
        glDisable(GL_LIGHTING);
        render_nodes();
        render_edges();
        glEnable(GL_LIGHTING);
    }

    void render_nodes(){
        Vector3VertexProperty vcoord = skel()->get_vertex_property<Vector3>("v:point");

        // Properties
        glColor3f(.5,.5,.5);
        glPointSize(4.0f);
        glEnable(GL_POINT_SMOOTH);

        glBegin(GL_POINTS);
        foreach(Vertex v, skel()->vertices())
            glVertex3dv(vcoord[v].data());
        glEnd();
    }

    void render_edges(){
        Vector3VertexProperty vcoord = skel()->get_vertex_property<Vector3>("v:point");

        glColor3f(1,0,0);

        float oldLineWidth = 0;
        glGetFloatv(GL_LINE_WIDTH, &oldLineWidth);
        glLineWidth(1.5f);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBegin(GL_LINES);
        foreach(Edge e, skel()->edges()){
            Vertex v0 = skel()->vertex(e,0);
            Vertex v1 = skel()->vertex(e,1);
            Vector3 p0 = vcoord[v0];
            Vector3 p1 = vcoord[v1];
            glColor3f(1.0f, 0.2f, 0.2f);
            glVertex3f(p0[0], p0[1], p0[2]);
            glColor3f(1.0f, 0.2f, 0.2f);
            glVertex3f(p1[0], p1[1], p1[2]);
        }
        glEnd();
        glDisable(GL_BLEND);

        glLineWidth(oldLineWidth);
    }
};

Renderer* plugin::instance(){return new CurveSkelLineRenderer();}

Q_EXPORT_PLUGIN(plugin)
