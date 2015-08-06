#include <qgl.h>
#include "plugin.h"
#include "surface_mesh/gl_wrappers.h"

using namespace SurfaceMesh;

// GLU was removed from Qt in version 4.8 
#ifdef Q_OS_MAC
# include <OpenGL/glu.h>
#else
# include <GL/glu.h>
#endif

class SortingTransparentRenderer : public SurfaceMeshRenderer{
private:
    ///< index array for buffered OpenGL rendering
    std::vector<unsigned int> triangles; 
    
private:
    typedef Surface_mesh::Face_iterator FaceItr;
    typedef std::pair<double, FaceItr> DepthFace;
    static bool depthSorter(DepthFace i, DepthFace j){ return (i.first < j.first); }
    
public:
    void render(){
        Surface_mesh::Vertex_property<Point>  points = mesh()->vertex_property<Point>("v:point");    
        Surface_mesh::Vertex_property<Point>  vnormals = mesh()->vertex_property<Point>("v:normal");
    
        // Deal with color
        bool has_vertex_color = mesh()->has_vertex_property<Color>("v:color");
        Surface_mesh::Vertex_property<Color>  vcolor;
        if (has_vertex_color) vcolor = mesh()->get_vertex_property<Color>("v:color");
    
        Surface_mesh::Vertex_around_face_circulator fvit, fvend;
    
        /// Sort faces
        std::vector< DepthFace > depthvec;
        {
            triangles.clear();
            triangles.resize(mesh()->n_faces());
    
            // Get current view transforms
            GLdouble projMatrix[16], modelMatrix[16];
            glGetDoublev(GL_PROJECTION_MATRIX, projMatrix);
            glGetDoublev(GL_MODELVIEW_MATRIX, modelMatrix);
    
            // Temporary variables for projection
            double depth,x,y;
            GLint viewport[4] = {0, 100, 100, -100};
    
            /// Apply model transform and fill in depth & indexes
            Surface_mesh::Face_iterator fit, fend = mesh()->faces_end();
            for (fit = mesh()->faces_begin(); fit != fend; ++fit){
    
                // Compute face center
                Vector3 faceCenter(0,0,0);
                fvit = fvend = mesh()->vertices(fit); int c = 0;
                do{ faceCenter += points[fvit]; c++; } while (++fvit != fvend);
                faceCenter /= c;
    
                // Project to get range 0 - 1.0
                gluProject(faceCenter.x(),faceCenter.y(),faceCenter.z(), modelMatrix, projMatrix, viewport,&x,&y,&depth);
    
                depthvec.push_back(DepthFace(depth, fit));
            }
    
            /// Sort depth 
            std::sort(depthvec.begin(), depthvec.end(), depthSorter);
        }
    
        /// Render
        {
            glShadeModel(GL_SMOOTH);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glEnable(GL_LIGHTING);
            glEnable(GL_NORMALIZE);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_CULL_FACE);
            {
                if(!has_vertex_color){
                    QColor& c = mesh()->color;
                    Eigen::Vector4d colorv(c.redF(), c.greenF(), c.blueF(), c.alphaF());
                    /// @todo minimum 10% transparency
                    colorv[3] = qMin(.1,colorv[3]);
                    glColor4dv(colorv.data());
                }
                glBegin(GL_TRIANGLES); 
                foreach(DepthFace f, depthvec)
                {
                    FaceItr fit = f.second;
                    fvit = fvend = mesh()->vertices(fit);
    
                    do{	
                        if(has_vertex_color){
                            // Fog like effect
                            Eigen::Vector4d colv(vcolor[fvit][0],vcolor[fvit][1],vcolor[fvit][2], 1 - f.first);
                            glColor4dv(colv.data());
                        }
    
                        gl::glNormal(vnormals[fvit]);
                        gl::glVertex(points[fvit]); 
                    } while (++fvit != fvend);
                }
                glEnd();
            }
            glEnable(GL_DEPTH_TEST);
            glEnable(GL_CULL_FACE);
        }
    }
};

Renderer* plugin::instance(){ return new SortingTransparentRenderer(); }

Q_EXPORT_PLUGIN(plugin)
