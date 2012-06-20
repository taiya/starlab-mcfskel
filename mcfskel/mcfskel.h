#pragma once
#pragma GCC diagnostic ignored "-Wwrite-strings"

#include <QDir>
#include "interfaces/SurfaceMeshModelPlugins.h"
#include "StarlabDrawArea.h"
#include "ColorizeHelper.h"
#include "CurvatureEstimationHelper.h"
#include "LaplacianHelper.h"
#include "StatisticsHelper.h"
#include "DoUndoHelper.h"
#include "LocalAnalysisHelper.h"
#include "SurfaceAreaHelper.h"

class mcfskel : public FilterPlugin{
    Q_OBJECT
    Q_INTERFACES(FilterPlugin)
    
public:
    virtual QString name() { return "Mesh Contraction"; }
    virtual QString description() { return "Performs Skeletonization by Collapsing Mesh Structure"; }
    virtual QKeySequence shortcut(){ return QKeySequence(Qt::CTRL + Qt::Key_K); }
    
public:    
    SurfaceMeshModel* mesh;
    StarlabDrawArea* drawArea;
    bool toggle; /// TRUE 
    bool firststep;
    Counter stepcount;
    mcfskel() : toggle(true), firststep(true), stepcount(0){}

public:
    void algorithm(RichParameterSet* );
    
public:
    void initParameters(Document* document, RichParameterSet* parameters, StarlabDrawArea* drawArea);
    void applyFilter(Document* document, RichParameterSet* pars, StarlabDrawArea* drawArea)
    {
        algorithm(pars);
    }
};

