/// @author Sofien Bouaziz 
/// @author Andrea Tagliasacchi

#pragma once
#include "SurfaceMeshPlugins.h"
using namespace SurfaceMesh;

class plugin : public SurfaceMeshFilterPlugin{
    Q_OBJECT
    Q_INTERFACES(FilterPlugin)

public:
    QString name() { return "Let the termites loose"; }
    QString description();
    void applyFilter(RichParameterSet*);
    void initParameters(RichParameterSet*);
};
