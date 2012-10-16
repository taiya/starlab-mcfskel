#pragma once
#include "SurfaceMeshPlugins.h"
class surfacemesh_filter_normalize : public SurfaceMeshFilterPlugin{
    Q_OBJECT
    Q_INTERFACES(FilterPlugin)

public:
    QString name() { return "Normalize Mesh"; }
    QString description() { return "Normalizes a mesh to unit bounding box"; }
    QKeySequence shortcut(){ return QKeySequence(Qt::CTRL + Qt::Key_N); }
    void applyFilter(RichParameterSet*);
};
