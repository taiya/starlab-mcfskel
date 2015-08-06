#pragma once
#include "SurfaceMeshPlugins.h"
class surfacemesh_io_off : public SurfaceMeshInputOutputPlugin{
    Q_OBJECT
    Q_INTERFACES(InputOutputPlugin)

public:
    QString name() { return "[SurfaceMesh] Object File Format (*.off)"; }
    Model *open(QString path);
    void save(SurfaceMeshModel *m, QString path);
};
