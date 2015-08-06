#pragma once
#include "SurfaceMeshPlugins.h"

class filter_au_skeleton : public FilterPlugin{
    Q_OBJECT
    Q_INTERFACES(FilterPlugin)

public:
    filter_au_skeleton(){}
    virtual QString name() { return "Mesh Contraction Skeleton"; }
    virtual QString description() { return "Skeleton Extraction by Mesh Contraction. Au et al. [SIGGRAPH 2008]"; }
    virtual void applyFilter(Document* document, RichParameterSet* parameters, StarlabDrawArea* drawArea);
    virtual void initParameters(Document* , RichParameterSet*, StarlabDrawArea*);

};
