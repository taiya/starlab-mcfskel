#pragma once
#include "CurveskelModel.h"
#include "interfaces/InputOutputPlugin.h"
#include "interfaces/FilterPlugin.h"
#include "interfaces/RenderPlugin.h"

using namespace CurveskelTypes;

/// Make this function valid only in this file
namespace{
    bool isA(Model* model){
        return (qobject_cast<CurveskelModel*>(model)!=NULL);
    }
    CurveskelModel* safeCast(Model* model){
        CurveskelModel* _model = qobject_cast<CurveskelModel*>(model);
        if(!_model) throw StarlabException("Model is not a SurfaceMeshModel");
        return _model;
    }
}

class CurveskelInputOutputPlugin : public InputOutputPlugin{
private:
    void save(Model* model,QString path){ save(safeCast(model),path); }
    bool isApplicable(Model* model){ return isA(model); }
public:
    virtual void save(CurveskelModel* model, QString path) = 0;
};

class CurveskelFilterPlugin : public FilterPlugin{
public:
    CurveskelModel* skel(){ return safeCast(model()); }
private:
    bool isApplicable(Model* model) { return isA(model); }
};

class CurveskelRenderPlugin : public RenderPlugin{
private:
    bool isApplicable(Model* model){ return isA(model); }
protected:
	CurveskelModel* skel() { return safeCast(model()); }
};
