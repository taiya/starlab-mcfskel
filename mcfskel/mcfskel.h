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

class mcfskel : public SurfaceMeshFilterPlugin{
    Q_OBJECT
    Q_INTERFACES(FilterPlugin)
    
public:
    QString name() { return "Mesh Contraction"; }
    QString description() { return "Performs Skeletonization by Collapsing Mesh Structure"; }
    QKeySequence shortcut(){ return QKeySequence(Qt::CTRL + Qt::Key_K); }
    
public:    
    bool toggle; /// TRUE 
    bool firststep;
    Counter stepcount;
    mcfskel() : toggle(true), firststep(true), stepcount(0){}

public:
    void initParameters(SurfaceMeshModel* /*model*/, RichParameterSet* /*parameters*/, StarlabDrawArea* /*drawArea*/);
    void applyFilter(SurfaceMeshModel* /*mesh*/, RichParameterSet* /*parameters*/, StarlabDrawArea* /*drawArea*/);
};

