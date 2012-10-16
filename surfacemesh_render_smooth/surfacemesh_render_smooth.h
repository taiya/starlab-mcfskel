#pragma once
#include "SurfaceMeshPlugins.h"

class surfacemesh_render_smooth : public SurfaceMeshRenderPlugin{
    Q_OBJECT
    Q_INTERFACES(RenderPlugin)

private:
    ///< index array for buffered OpenGL rendering
    std::vector<unsigned int> triangles; 
    
public: 
    RenderPlugin* factory(){ return new surfacemesh_render_smooth(); }
    QString name() { return "Smooth Shading"; }
    QIcon icon(){ return QIcon(":/surfacemesh_render_smooth.png"); }
    void init();
    void render();

protected:
    virtual bool isDefault() { return true; }
};
