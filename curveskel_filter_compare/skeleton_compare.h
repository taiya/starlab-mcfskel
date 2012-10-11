#pragma once
#include "CurveskelModel.h"
#include "CurveskelPlugins.h"

using namespace CurveskelTypes;

class skeleton_compare : public CurveskelFilterPlugin{
    Q_OBJECT
    Q_INTERFACES(FilterPlugin)

private:    
    QMap<QString, CurveskelModel*> skeletons;

public:
    QString name() { return "Skeleton compare"; }
    QString description() { return "Computes difference between skeletons"; }
    
    void initParameters(RichParameterSet* pars);
    void applyFilter(RichParameterSet*);
};
