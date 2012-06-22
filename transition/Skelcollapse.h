#pragma once
#pragma GCC diagnostic ignored "-Wwrite-strings"

#include <QDir>
#include "interfaces/SurfaceMeshModelPlugins.h"
#include "StarlabDrawArea.h"
#include "ColorizeHelper.h"
#include "CurvatureEstimationHelper.h"
#include "LegacyLaplacianHelper.h"
#include "StatisticsHelper.h"
#include "DoUndoHelper.h"
#include "LocalAnalysisHelper.h"
#include "MatlabLaplacianHelper.h"
#include "SurfaceAreaHelper.h"

class Skelcollapse : public FilterPlugin{
    Q_OBJECT
    Q_INTERFACES(FilterPlugin)
    
public:
    virtual QString name() { return "Mesh Contraction (Modding)"; }
    virtual QString description() { return "Performs Skeletonization by Collapsing Mesh Structure"; }
    virtual QKeySequence shortcut(){ return QKeySequence(Qt::CTRL + Qt::Key_L); }
    
public:
    void algorithm(RichParameterSet* );
    
public:
    void initParameters(Document* document, RichParameterSet* parameters, StarlabDrawArea* drawArea){
        SurfaceMeshModel* mesh = qobject_cast<SurfaceMeshModel*>(document->selectedModel());
    
        /// Add a transparent copy of the model
        {
            SurfaceMeshModel* copy = new SurfaceMeshModel(mesh->path,"original");
            copy->read( mesh->path.toStdString() );
            document->addModel(copy);
            drawArea->initRendering();
            copy->renderer()->setRenderMode("Transparent"); 
        }
            
        Scalar scale = 0.002*mesh->getBoundingBox().size().length();
        parameters->addParam(new RichFloat("omega_L_0",1));
        parameters->addParam(new RichFloat("omega_H_0",20));
        parameters->addParam(new RichFloat("omega_P_0",40));
        parameters->addParam(new RichFloat("edgelength_TH",scale));
        parameters->addParam(new RichFloat("alpha",0.15));
        parameters->addParam(new RichFloat("zero_TH",1e-10));
    }

    void applyFilter(Document* document, RichParameterSet* pars, StarlabDrawArea* drawArea);
};
