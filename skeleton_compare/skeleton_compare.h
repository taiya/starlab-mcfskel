#pragma once
#include "interfaces/FilterPlugin.h"
#include "SkeletonModel.h"

class skeleton_compare : public FilterPlugin{
    Q_OBJECT
    Q_INTERFACES(FilterPlugin)

private:    
    QMap<QString, SkeletonModel*> skeletons;

public:
    QString name() { return "Skeleton compare"; }
    QString description() { return "Computes difference between skeletons"; }
    
    void initParameters(Document* doc, RichParameterSet* pars, StarlabDrawArea *){
        /// QMap of skeleton models indexed by name
        skeletons.clear();
        foreach(Model* model, doc->getModels()){
            SkeletonModel* skel = qobject_cast<SkeletonModel*>(model);
            if(skel) skeletons.insert(model->name, skel);
        }
        if(skeletons.size()<2) 
            throw StarlabException("Comparison requires TWO pre-loaded skeleton models");
        
        pars->addParam( new RichStringSet("Target skeleton", skeletons.keys(), "Target skeleton","The skeleton model to compare *toward*") );
        pars->addParam( new RichBool("Show measurements", false, "Show measurements", "Show blue edge connecting corresponding points") );
    }

    void applyFilter(Document*, RichParameterSet*, StarlabDrawArea*);
};
