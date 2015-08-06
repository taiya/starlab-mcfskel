#pragma once
#include "SurfaceMeshPlugins.h"
class surfacemesh_io_obj : public SurfaceMeshInputOutputPlugin{
    Q_OBJECT
    Q_INTERFACES(InputOutputPlugin)

public:
    QString name() { return "[SurfaceMesh] Wavefront Object (*.obj)"; }
    Model* open(QString path);
    void save(SurfaceMeshModel*, QString);
};
