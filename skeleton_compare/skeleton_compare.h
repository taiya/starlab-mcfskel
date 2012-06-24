#pragma once
#include "interfaces/FilterPlugin.h"
class skeleton_compare : public FilterPlugin{
    Q_OBJECT
    Q_INTERFACES(FilterPlugin)

public:
    QString name() { return "Skeleton compare"; }
    QString description() { return "Computes difference between skeletons"; }

    void initParameters(Document *, RichParameterSet* /*pars*/, StarlabDrawArea *);
    void applyFilter(Document*, RichParameterSet*, StarlabDrawArea*);
};
