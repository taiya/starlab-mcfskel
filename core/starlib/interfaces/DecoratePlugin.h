#pragma once 
#include "StarlabPlugin.h"

/**
 * Decorate your rendering with extra stuff (i.e. normals, selection, etc..)
 * @ingroup stariface 
 */
class STARLIB_EXPORT DecoratePlugin : public StarlabPlugin{
/// @{ DecoratePlugin interface
public:
    /// Overload it and return a new instance of your plugin    
    virtual DecoratePlugin* factory()=0;  
    
    /// Given the current Model, can the plugin be executed? 
    virtual bool isApplicable(Starlab::Model*) = 0;
    
    /// This is similar to a render loop, thus it's called repeteadly
    virtual void decorate() = 0;
    
    /// Constructor (allocate resources)
    virtual void create(){}
    
    /// Destructor (free resources)
    virtual void destroy() {}
    
    /// returns the model that this plugin should be decorating
    Starlab::Model* model(){ Q_ASSERT(_model!=NULL); return _model; }
    
    /// Allows access to this->drawArea()
    using StarlabPlugin::drawArea;
/// @}    

/// @{ Internal Usage
public:
    /// @internal A decorator plugin can either be enabled or disabled thus we make its action "Checkable"
    QAction* action(){
        QAction* action = StarlabPlugin::action();
        action->setCheckable(true);
        action->setChecked(false);
        return action;
    }
private:
    /// @internal Allow it to set _model
    friend class Starlab::Model;
    /// @internal The model that this plugin is decorating
    Starlab::Model* _model;
/// @}
};

Q_DECLARE_INTERFACE(DecoratePlugin, "Starlab::DecoratePlugin/2.0")
