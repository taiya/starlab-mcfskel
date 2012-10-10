#pragma once
#include "SurfaceMeshPlugins.h"

class LaplacianRemesher : public SurfaceMeshFilterPlugin{
    Q_OBJECT
    Q_INTERFACES(FilterPlugin)

public:
    QString name() { return "Remeshing/Laplacian"; }
    QString description() { return "Remeshes the surface by applying tangential flow, edge splits, collapses and flips"; }
    void applyFilter(RichParameterSet*);
    void initParameters(RichParameterSet* parameters);
};
