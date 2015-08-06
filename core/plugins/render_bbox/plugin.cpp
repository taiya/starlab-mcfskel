#include "plugin.h"
Q_EXPORT_PLUGIN(plugin);

#include <QtOpenGL>

class RenderBBOX : public Renderer{
public:
    void render(){
        /// Set line width
        /// @todo this should become a parameter
        glLineWidth(1.0f);
        
        /// Setup BBOX color
        Eigen::AlignedBox3d bbox = model()->bbox();
        QColor& c = model()->color;
        glColor3f(c.redF(),c.greenF(),c.blueF());
        
        float min[3]; 
        min[0] = bbox.min().x();
        min[1] = bbox.min().y();
        min[2] = bbox.min().z();
    
        float max[3]; 
        max[0] = bbox.max().x();
        max[1] = bbox.max().y();
        max[2] = bbox.max().z();
            
        /// --- Inherited from VCG ---
        glPushAttrib(GL_ENABLE_BIT);
        glDisable(GL_LIGHTING);
        glBegin(GL_LINE_STRIP);
        glVertex3f((float)min[0],(float)min[1],(float)min[2]);
        glVertex3f((float)max[0],(float)min[1],(float)min[2]);
        glVertex3f((float)max[0],(float)max[1],(float)min[2]);
        glVertex3f((float)min[0],(float)max[1],(float)min[2]);
        glVertex3f((float)min[0],(float)min[1],(float)min[2]);
        glEnd();
        glBegin(GL_LINE_STRIP);
        glVertex3f((float)min[0],(float)min[1],(float)max[2]);
        glVertex3f((float)max[0],(float)min[1],(float)max[2]);
        glVertex3f((float)max[0],(float)max[1],(float)max[2]);
        glVertex3f((float)min[0],(float)max[1],(float)max[2]);
        glVertex3f((float)min[0],(float)min[1],(float)max[2]);
        glEnd();
        glBegin(GL_LINES);
        glVertex3f((float)min[0],(float)min[1],(float)min[2]);
        glVertex3f((float)min[0],(float)min[1],(float)max[2]);
        glVertex3f((float)max[0],(float)min[1],(float)min[2]);
        glVertex3f((float)max[0],(float)min[1],(float)max[2]);
        glVertex3f((float)max[0],(float)max[1],(float)min[2]);
        glVertex3f((float)max[0],(float)max[1],(float)max[2]);
        glVertex3f((float)min[0],(float)max[1],(float)min[2]);
        glVertex3f((float)min[0],(float)max[1],(float)max[2]);
        glEnd();
        glPopAttrib();    
    }
};

Renderer* plugin::instance(){ return new RenderBBOX(); }
