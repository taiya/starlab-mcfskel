#pragma once
#include "SurfaceMeshPlugins.h"

class surfacemesh_render_flatwire : public SurfaceMeshRenderPlugin{
    Q_OBJECT
    Q_INTERFACES(RenderPlugin)

private:
    /// index array for buffered OpenGL rendering
    std::vector<unsigned int> edges;
    std::vector<unsigned int> triangles; 

public: 
    RenderPlugin* factory(){ return new surfacemesh_render_flatwire(); }
    QString name() { return "Flat Wire"; }
    QIcon icon(){ return QIcon(":/surfacemesh_render_flatwire.png"); }
    void init();
    void render();

private:
    void render_flat();
    void render_wire();
};
