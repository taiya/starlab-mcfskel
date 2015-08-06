#pragma once 
#include "SurfaceMeshPlugins.h"

class surfacemesh_decorate_normals : public SurfaceMeshDecoratePlugin{
    Q_OBJECT
    Q_INTERFACES(DecoratePlugin)

public:
    virtual QString name() { return "Toggle show normals"; }
    virtual QString description() { return "Performs Skeletonization by Collapsing Mesh Structure"; }
    void decorate();
};
