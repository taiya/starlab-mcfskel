/// @author Sofien Bouaziz 
/// @author Andrea Tagliasacchi

#pragma once
#include "SurfaceMeshPlugins.h"
using namespace SurfaceMesh;

class filter_icp : public SurfaceMeshFilterPlugin{
    Q_OBJECT
    Q_INTERFACES(FilterPlugin)

public:
    QString name() { return "Iterative Closest Point"; }
    void applyFilter(RichParameterSet*);
    void initParameters(RichParameterSet*);
};
