#pragma once
#include "SurfaceMeshPlugins.h"

class surfacemesh_render_transparent : public SurfaceMeshRenderPlugin{
    Q_OBJECT
    Q_INTERFACES(RenderPlugin)

private:
    ///< index array for buffered OpenGL rendering
    std::vector<unsigned int> triangles; 
    
public: 
    RenderPlugin* factory(){ return new surfacemesh_render_transparent(); }
    QString name() { return "Transparent"; }
    QIcon icon(){ return QIcon(":/icons/surfacemesh_render_transparent.png"); }
    void render();
};
