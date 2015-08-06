#include "plugin.h"
Q_EXPORT_PLUGIN(surfacemesh_render_wireframe)

#include <QElapsedTimer>
#include <qgl.h>
#include "surface_mesh/gl_wrappers.h"

using namespace Eigen;

class FlatRenderer : public SurfaceMeshRenderer{
    /// Declaring all buffers
    std::vector<unsigned int> triangles;
    std::vector<double> vbo_flat;
    std::vector<double> fnormal_flat;
    std::vector<double> fcolor_flat;
    bool has_face_color;


    void init(){
        /// Clearing all buffers
        triangles.clear();
        vbo_flat.clear();
        fnormal_flat.clear();
        fcolor_flat.clear();

        /// updating mesh normals
        mesh()->update_face_normals();

        /// valence[v] gives the number of faces v is present in
        Surface_mesh::Vertex_property<int> valence = mesh()->vertex_property<int>("v:valence");
        Surface_mesh::Vertex_property<int> counter = mesh()->vertex_property<int>("v:counter");

        /// cumulativeValence[v] gives the sum of valence[0] to valence[v-1]
        Surface_mesh::Vertex_property<int> cumulativeValence = mesh()->vertex_property<int>("v:cumulativeValence");

        Surface_mesh::Vertex_property<Point> points = mesh()->vertex_property<Point>(VPOINT);
        Surface_mesh::Face_property<Point> fnormal = mesh()->face_property<Point>("f:normal");
        Surface_mesh::Face_property<Color> fcolor = mesh()->get_face_property<Color>("f:color");
        has_face_color = mesh()->has_face_property<Color>("f:color");

        /// Initialising the fnormal_flat and fcolor_flat buffers to 0
        fnormal_flat.resize(mesh()->n_faces()*3*3);
        if(has_face_color) fcolor_flat.resize(mesh()->n_faces()*3*3);

        /// Initialising valence and counter to 0
        foreach(Vertex v, mesh()->vertices()){
            valence[v]=0;
            counter[v]=0;
        }


        /// Calculating valence of each vertex
        foreach(Face f, mesh()->faces()){
            foreach(Vertex v, mesh()->vertices(f)){
                valence[v]++;
            }
        }

        /// Calculating cumulative valence
        int val=0;
        foreach(Vertex v, mesh()->vertices()){
            cumulativeValence[v]=val;
            val+=valence[v];
        }

        /// Initialising vertex buffer
        double *vpointer, vcoord[3];
        foreach(Vertex v, mesh()->vertices()){
            vpointer=points[v].data();
            vcoord[0]=*vpointer; vpointer++;
            vcoord[1]=*vpointer; vpointer++;
            vcoord[2]=*vpointer;
            for(int i=0; i<valence[v]; i++){
                vbo_flat.push_back(vcoord[0]);
                vbo_flat.push_back(vcoord[1]);
                vbo_flat.push_back(vcoord[2]);
            }
        }

        /// Inserting values into fnormal_flat, fcolor_flat and the triangles buffer
        int idx_new;
        double *fpointer, *colorpointer, fcoord[3], colorCoord[3];
        foreach(Face f, mesh()->faces()){
            fpointer = fnormal[f].data();
            fcoord[0]=*fpointer; fpointer++;
            fcoord[1]=*fpointer; fpointer++;
            fcoord[2]=*fpointer;

            if(has_face_color){
                colorpointer = fcolor[f].data();
                colorCoord[0]=*colorpointer; colorpointer++;
                colorCoord[1]=*colorpointer; colorpointer++;
                colorCoord[2]=*colorpointer;
            }

            foreach(Vertex v, mesh()->vertices(f)){
                idx_new = cumulativeValence[v]*3+counter[v]*3;
                fnormal_flat.at(idx_new) = fcoord[0];
                fnormal_flat.at(idx_new+1) = fcoord[1];
                fnormal_flat.at(idx_new+2) = fcoord[2];
                if(has_face_color){
                    fcolor_flat.at(idx_new) = colorCoord[0];
                    fcolor_flat.at(idx_new+1) = colorCoord[1];
                    fcolor_flat.at(idx_new+2) = colorCoord[2];
                }

                triangles.push_back(cumulativeValence[v]+counter[v]);

                counter[v]++;
            }
        }
    }
    
    void render(){

        if( vbo_flat.empty() ) return;

        gl::glVertexPointer(3, GL_DOUBLE, 0, &vbo_flat[0]);
        gl::glNormalPointer(GL_DOUBLE, 0, &fnormal_flat[0]);
        if(has_face_color)
            gl::glColorPointer(3, GL_DOUBLE, 0, &fcolor_flat[0]);

        glEnable(GL_LIGHTING);
        glShadeModel(GL_FLAT);
        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_NORMAL_ARRAY);
        if(has_face_color) glEnableClientState(GL_COLOR_ARRAY);
        if(triangles.size()) glDrawElements(GL_TRIANGLES, (GLsizei)triangles.size(), GL_UNSIGNED_INT, &triangles[0]);
        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_NORMAL_ARRAY);

#if 0
        Surface_mesh::Vertex_property<Point> points   = mesh()->vertex_property<Point>("v:point");
        Surface_mesh::Face_property<Point>   fnormals = mesh()->face_property<Point>("f:normal");
        Surface_mesh::Face_property<Color>   fcolor   = mesh()->get_face_property<Color>("f:color");
        bool has_face_color                           = mesh()->has_face_property<Color>("f:color");

        // setup vertex arrays
        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_NORMAL_ARRAY);
        glEnable(GL_LIGHTING);
        glShadeModel(GL_FLAT);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glDepthRange(0.01, 1.0); ///< @todo what is this for?

        /// Set the color
        glColor4d(mesh()->color.redF(), mesh()->color.greenF(), mesh()->color.blueF(), mesh()->color.alphaF());

        /// Render loop  @todo convert to buffers
        foreach(Face f, mesh()->faces()){
            glBegin(GL_POLYGON);
                if(has_face_color)
                    gl::glColor(fcolor[f]);
                gl::glNormal(fnormals[f]);
                foreach(Vertex v, mesh()->vertices(f))
                    gl::glVertex(points[v]);
            glEnd();
        }
#endif
        
#if 0 
        /// Older code not using foreach
        QElapsedTimer timer;
        timer.restart();
        /// @todo convert to "foreach"
        Surface_mesh::Face_iterator fit, fend=mesh()->faces_end();
        Surface_mesh::Vertex_around_face_circulator fvit, fvend;
        for (fit=mesh()->faces_begin(); fit!=fend; ++fit){
            // this will be slow, but we don't want to distinguish between
            // triangles, quads, and general polygons
            // immediate mode rendering is slow anyway...
            glBegin(GL_POLYGON);
                if(has_face_color) gl::glColor(fcolor[fit]);
                gl::glNormal(fnormals[fit]);
                fvit = fvend = mesh()->vertices(fit);
                do
                {
                    gl::glVertex(points[fvit]);
                }
                while (++fvit != fvend);
            glEnd();
        }
        qDebug() << "TRADITIONAL" << timer.elapsed();
#endif
    }
};

Renderer *surfacemesh_render_wireframe::instance(){ return new FlatRenderer(); }

