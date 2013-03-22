#pragma once
#include "CurveskelModel.h"
#include "interfaces/InputOutputPlugin.h"
#include "interfaces/FilterPlugin.h"
#include "interfaces/RenderPlugin.h"
#include "parameters/RichParameterSet.h"

using namespace CurveskelTypes;

/// Make this function valid only in this file
namespace{
    bool isA(Starlab::Model* model){
        return (qobject_cast<CurveskelModel*>(model)!=NULL);
    }
    CurveskelModel* safeCast(Starlab::Model* model){
        CurveskelModel* _model = qobject_cast<CurveskelModel*>(model);
        if(!_model) throw StarlabException("Model is not a SurfaceMeshModel");
        return _model;
    }
}

class CurveskelInputOutputPlugin : public InputOutputPlugin{
private:
    void save(Starlab::Model* model,QString path){ save(safeCast(model),path); }
    bool isApplicable(Starlab::Model* model){ return isA(model); }
public:
    virtual void save(CurveskelModel* model, QString path) = 0;
};

class CurveskelFilterPlugin : public FilterPlugin{
public:
    CurveskelModel* skel(){ return safeCast(model()); }
private:
    bool isApplicable(Starlab::Model* model) { return isA(model); }
};

class CurveskelModelRenderer : public Renderer{
public:
    CurveskelModel* skel() { return safeCast(model()); }
};

class CurveskelRenderPlugin : public RenderPlugin{
private:
    bool isApplicable(Starlab::Model* model){ return isA(model); }
};
