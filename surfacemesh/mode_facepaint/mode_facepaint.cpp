#include "mode_facepaint.h"
Q_EXPORT_PLUGIN(mode_facepaint)

#include <QtOpenGL>
#include "StarlabDrawArea.h"
#include "FaceBarycenterHelper.h"

using namespace Starlab;

void mode_facepaint::create(){
    _button = Qt::NoButton;
    mesh = SurfaceMesh::safeCast(selection());
    vpoints = mesh->vertex_coordinates();
    fselect = mesh->add_face_property<bool>(FSELECTED,false);
    fbarycenter = FaceBarycenterHelper(mesh);
    
    // drawArea()->setRenderer(mesh,SHADING::FLAT);
}

bool mode_facepaint::mouseReleaseEvent(QMouseEvent* ){
    _button = Qt::NoButton;   
    return true;
}

bool mode_facepaint::mousePressEvent(QMouseEvent * event){   
    _button = event->button();
    return true;
}

bool mode_facepaint::mouseMoveEvent(QMouseEvent *event ){          
    if(_button != Qt::NoButton){
        drawArea()->select( event->pos() ); 
        /// "draw_with_names()" called here
        int foffset = drawArea()->selectedName();
        if(foffset>0 && foffset< (int) mesh->n_faces()){
            Face face(foffset);
            
            switch(_button){
                case Qt::RightButton: fselect[face] = false; break;
                case Qt::LeftButton:  fselect[face] = true; break;
                default: break;
            }

            // using decorate() instead as now removal is supported
            // drawArea()->drawPoint(fbarycenter[face]).color(Qt::red).size(5);
            drawArea()->updateGL();
        }
    }
    return true;
}

void mode_facepaint::drawWithNames(){
    glInitNames();
    foreach(Face f, mesh->faces()){
        glPushName(f.idx());
            glBegin(GL_POLYGON);
                foreach(Vertex v, mesh->vertices(f))
                    glVertex3dv(vpoints[v].data());
            glEnd();
        glPopName();
    }
}

void mode_facepaint::decorate(){
    glDisable(GL_LIGHTING);

    foreach(Face f, mesh->faces()){
        if( !fselect[f] ) 
            continue;
        Starlab::Vector3 fbary = fbarycenter[f];
        glPointSize(5);
        glColor3d(1.0,0.0,0.0);
        glBegin(GL_POINTS);
            glVertex3dv(fbary.data());
        glEnd();
    }

    glEnable(GL_LIGHTING);
}
