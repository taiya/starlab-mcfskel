#pragma once
#include "interfaces/FilterPlugin.h"
class voromat : public FilterPlugin{
    Q_OBJECT
    Q_INTERFACES(FilterPlugin)

public:
    QString name() { return "Voronoi based MAT"; }
    QString description() { return "Computes voronoi embedding of a surface"; }
    
    void initParameters(Document *, RichParameterSet* /*pars*/, StarlabDrawArea *){
        /// @todo range 0...90deg, for multi-scale medial feature
        // pars->addParam(new RichFloat("Angle",90.0f,"Discarded medial spokes"));
        /// @todo compute it proportionally to average edge length
        // pars->addParam(new RichFloat("Distance",0.01f,"Discarded medial spokes"));
    }    
    
    void applyFilter(Document*, RichParameterSet*, StarlabDrawArea*);
};
