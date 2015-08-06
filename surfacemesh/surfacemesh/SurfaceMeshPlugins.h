/// @todo this file should be split in a number of files inside the folder interfaces/
/// otherwise you will end up having to recompile all the surfacemeshplugins when it changes...

#pragma once
#include "InputOutputPlugin.h"
#include "DecoratePlugin.h"
#include "FilterPlugin.h"
#include "ModePlugin.h"
#include "RenderPlugin.h"
#include "RichParameterSet.h"
#include "SurfaceMeshModel.h"

/// Legal here as this is included in plugins
using namespace SurfaceMesh;

class SurfaceMeshInputOutputPlugin : public InputOutputPlugin{
private: 
    void save(Starlab::Model* model,QString path){ save(safeCast(model),path); }
    bool isApplicable(Starlab::Model* model){ return isA(model); }    
public:
    virtual void save(SurfaceMesh::Model* model, QString path) = 0;
};

class SurfaceMeshRenderer : public Renderer{
public:
    SurfaceMeshModel* mesh(){ return safeCast(model()); }
};

class SurfaceMeshRenderPlugin : public RenderPlugin{
private: 
    bool isApplicable(Starlab::Model* model){ return isA(model); }
};

class SurfaceMeshFilterPlugin : public FilterPlugin{
public:
    SurfaceMesh::Model* mesh(){ return safeCast(model()); }    
private:
    bool isApplicable(Starlab::Model* model) { return (model!=NULL) && isA(model); }
};

class SurfaceMeshModePlugin : public ModePlugin{
public:
    SurfaceMesh::Model* mesh(){ return safeCast(document()->selectedModel()); }
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
