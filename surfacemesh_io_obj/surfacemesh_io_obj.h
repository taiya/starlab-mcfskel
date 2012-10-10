#pragma once
#include "SurfaceMeshPlugins.h"

class surfacemesh_io_obj : public SurfaceMeshInputOutputPlugin{
    Q_OBJECT
    Q_INTERFACES(InputOutputPlugin)

public:
    QString name() { return "[MCFSkel] Wavefront Object (*.obj)"; }
    void save(SurfaceMeshModel* mesh, QString path);
    Model* open(QString path);
};
