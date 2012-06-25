#pragma once
#include "interfaces/FilterPlugin.h"
#include "SkeletonModel.h"
#include "helpers/QForEachHelpers.h"

class skeleton_resample : public SelectionFilterPlugin{
    Q_OBJECT
    Q_INTERFACES(FilterPlugin)

private:
    SkeletonModel * skel;
    SkeletonModel::Vertex_property<SkeletonTypes::Point> skel_points;

    void recursiveSplitEdge(SkeletonModel::Edge e, double threshold);

public:
    QString name() { return "Skeleton resampler"; }
    QString description() { return "Skeleton resampling by mid-point edge split."; }
	void applyFilter(Model*,RichParameterSet*, StarlabDrawArea*);
    void initParameters(Document *document, RichParameterSet *parameters, StarlabDrawArea *drawArea);
};
