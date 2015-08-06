#pragma once
#include "SurfaceMeshPlugins.h"
class surfacemesh_filter_flattenlayers : public SurfaceMeshFilterPlugin{
    Q_OBJECT
    Q_INTERFACES(FilterPlugin)

public:
    QString name() { return "Flatten Layers"; }
    QString description() { return "Merges two layers into a single one"; }
    void initParameters(RichParameterSet* pars);
    void applyFilter(RichParameterSet*);
};
