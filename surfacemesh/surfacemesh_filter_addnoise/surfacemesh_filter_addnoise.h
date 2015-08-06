#pragma once
#include "SurfaceMeshPlugins.h"
class surfacemesh_filter_addnoise : public SurfaceMeshFilterPlugin{
    Q_OBJECT
    Q_INTERFACES(FilterPlugin)

public:
    QString name() { return "+Noise"; }
    QString description() { return "Adds noise to the model"; }
    void initParameters(RichParameterSet* pars);
    void applyFilter(RichParameterSet*);
};
