#pragma once
#include "StarlabPlugin.h"
#include "RichParameterSet.h"
#include "Model.h"

class RenderPlugin;

class STARLIB_EXPORT Renderer : public QObject{
    Q_OBJECT
public slots:
    /// @brief Initialize render plugin (optional), for example
    /// - initialize GL memory buffer
    /// - read shader specification from file
    /// @todo rename into create
    virtual void init(){}

    /// @brief Render the model, default implementation only sets glColor
    virtual void render() = 0;
    
    /// @brief Initialize the parameters of your plugin here, see parameters()
    virtual void initParameters(){}    
    
    /// @brief Clean up resources (the ones you allocated in init())
    /// @todo rename into destroy
    virtual void finalize(){}    
    
public:
    /// @brief returns the model that this plugin should be rendering
    StarlabModel* model(){ return _model; }
    /// @brief returns the plugin that instantiated this renderer
    RenderPlugin* plugin(){ Q_ASSERT(_plugin); return _plugin; }
    /// @brief returns the parameter set you can edit
    RichParameterSet* parameters(){ return &_parameters; }
    
private:
    friend class Starlab::Model;
    /// the model to whom this plugin applies
    StarlabModel* _model;
    /// keeps track of who generated this renderer
    RenderPlugin* _plugin; 
    /// renderer parameters
    RichParameterSet _parameters;
};

class RenderPlugin : public StarlabPlugin{
public:
    /// @brief Can this plugin generate a renderer for the the given model?
    virtual bool isApplicable(StarlabModel* model) = 0;
    /// @brief Generate an instance of the rendering class
    virtual Renderer* instance() = 0;
    /// @brief Overload and return true if you would like this plugin to be a default
    virtual bool isDefault() { return false; }   
    /// @brief Gives access to the StarlabDrawArea
    using StarlabPlugin::drawArea;
};

Q_DECLARE_INTERFACE(RenderPlugin, "Starlab::RenderPlugin/3.0")
