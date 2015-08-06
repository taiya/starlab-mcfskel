#pragma once
#include "SurfaceMeshPlugins.h"
using namespace Starlab;

class surfacemesh_render_wireframe : public SurfaceMeshRenderPlugin{
    Q_OBJECT
    Q_INTERFACES(RenderPlugin)

    QString name() { return SHADING::FLAT; }
    QIcon icon(){ return QIcon(":/icons/flat_shading.png"); }
    Renderer* instance();
};
