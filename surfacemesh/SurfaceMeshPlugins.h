/// @todo this file should be split in a number of files inside the folder interfaces/
/// otherwise you will end up having to recompile all the surfacemeshplugins when it changes...

#pragma once
#include "interfaces/InputOutputPlugin.h"
#include "interfaces/DecoratePlugin.h"
#include "interfaces/FilterPlugin.h"
#include "interfaces/ModePlugin.h"
#include "interfaces/RenderPlugin.h"
#include "parameters/RichParameterSet.h"

#include "SurfaceMeshModel.h"
#include "SurfaceMeshTypes.h"

using namespace SurfaceMeshTypes;

namespace{
    /// Is the given model a SurfaceMeshModel?
    bool isA(Model* model){ return qobject_cast<SurfaceMeshModel*>(model); }
    /// Safely convert to a surfacemesh
    SurfaceMeshModel* safeCast(Model* model){
        SurfaceMeshModel* mesh = qobject_cast<SurfaceMeshModel*>(model);
        if(!mesh) throw StarlabException("Model is not a SurfaceMeshModel");
        return mesh;
    }
}

class SurfaceMeshInputOutputPlugin : public InputOutputPlugin{
private: 
    void save(Model* model,QString path){ save(safeCast(model),path); }
    bool isApplicable(Model* model){ return isA(model); }    
public:
    virtual void save(SurfaceMeshModel* model, QString path) = 0;
};

class SurfaceMeshRenderPlugin : public RenderPlugin{
private: 
    bool isApplicable(Model* model){ return isA(model); }
public: 
    SurfaceMeshModel* mesh(){ return safeCast(model()); }
};

class SurfaceMeshFilterPlugin : public FilterPlugin{
public:
    SurfaceMeshModel* mesh(){ return safeCast(model()); }    
private:
    bool isApplicable(Model* model) { return isA(model); }
};

class SurfaceMeshModePlugin : public ModePlugin{
public:
    SurfaceMeshModel* mesh(){ return safeCast(document()->selectedModel()); }
private:
    bool isApplicable() { return isA(document()->selectedModel()); }
};

#if 0
class SurfaceMeshDecoratePlugin : public DecoratePlugin{
public:
    SurfaceMeshModel* mesh(){ return safeCast(document()->selectedModel()); }    
private:
    bool isApplicable(Model* model) { return isA(model); }
};
#endif
