#pragma once
#include "SurfaceMeshPlugins.h"
#include "SurfaceMeshHelper.h"
#include "RichParameterSet.h"

#include "GeoHeatHelper.h"

// This plugin efficently computes shortest geodesic distance to points on the mesh
// It is based on the paper "Geodesics in Heat" by Kennan Crane et al. 2012

class surfacemesh_filter_geoheat : public SurfaceMeshFilterPlugin{
    Q_OBJECT
    Q_INTERFACES(FilterPlugin)

public:
    surfacemesh_filter_geoheat() { h = NULL; }
    ~surfacemesh_filter_geoheat(){ delete h; }
    QString name() { return "Geodesic distance: heat kernel"; }
    QString description() { return "Compute geodesic distance using the Heat Kernel."; }

    void initParameters(RichParameterSet* pars);
    void applyFilter(RichParameterSet* pars);

    GeoHeatHelper * h;
};
