#include "surfacemesh_decorate_normals.h"
#include <QtOpenGL>
#include "StarlabDrawArea.h"
#include "SurfaceMeshModel.h"
#include "SurfaceMeshTypes.h"
#include "SurfaceMeshHelper.h"

using namespace SurfaceMeshTypes;

/// This is similar to a render loop, thus it's called repeteadly
void surfacemesh_decorate_normals::decorate(){
    float NormalLen =.05; /// @todo read from setting
    float LineLen = mesh()->bbox().size().length() * NormalLen;    
    
    /// Pre-computation to be moved to update area
    SurfaceMeshHelper smh(mesh());
    Vector3FaceProperty fb = smh.computeFaceBarycenters(FBARYCENTER);
    Vector3FaceProperty fn = smh.computeFaceNormals(FNORMAL);
    Vector3FaceProperty fo = smh.vector3FaceProperty("f:boffset",Vector3(0));
    foreach(Face f, mesh()->faces())
        fo[f] = fb[f] + fn[f]*LineLen;
    
    /// Draw area
    glPushAttrib(GL_ENABLE_BIT );
        glDisable(GL_LIGHTING);
        glDisable(GL_TEXTURE_2D);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
        glBegin(GL_LINES);
            glColor4f(.1f,.4f,4.f,.6f);   
            foreach(Face f, mesh()->faces()){
                glVertex3dv( fb[f].data() );
                glVertex3dv( fo[f].data() );            
            }
        glEnd();
    glPopAttrib();
}

Q_EXPORT_PLUGIN(surfacemesh_decorate_normals)
