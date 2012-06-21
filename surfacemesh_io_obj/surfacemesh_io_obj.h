#pragma once
#include "interfaces/SurfaceMeshModelPlugins.h"

class surfacemesh_io_obj : public SurfaceMeshInputOutputPlugin{
    Q_OBJECT
    Q_INTERFACES(InputOutputPlugin)

public:
    QString name() { return "[MCFSkel] Wavefront Object (*.obj)"; }
    void save(QString path, SurfaceMeshModel* mesh);
    Model* open(QString path);
};
