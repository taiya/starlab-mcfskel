#pragma once
#include "SurfaceMeshPlugins.h"

class plugin : public SurfaceMeshRenderPlugin{
    Q_OBJECT
    Q_INTERFACES(RenderPlugin)
   
public: 
    QString name() { return "Wireframe"; }
    QIcon icon(){ return QIcon(":/icons/wireframe.png"); }
    Renderer* instance();
};
