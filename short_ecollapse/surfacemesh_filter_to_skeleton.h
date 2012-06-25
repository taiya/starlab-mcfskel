#pragma once
#include "interfaces/SurfaceMeshModelPlugins.h"

class surfacemesh_filter_to_skeleton : public FilterPlugin{
    Q_OBJECT
    Q_INTERFACES(FilterPlugin)

public:
    QString name() { return "SurfaceMesh to Skeleton"; }
    QString description() { return "Converts a SurfaceMeshModel into a SkeletonModel"; }
    void applyFilter(Document* document, RichParameterSet* parameters, StarlabDrawArea* drawArea);
    bool isApplicable(Document* document){ return qobject_cast<SurfaceMeshModel*>(document->selectedModel()); }
};
