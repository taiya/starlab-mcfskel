#pragma once 
#include "StarlabPlugin.h"

/** 
 * @brief The interface of a plugin which adds GUI functionality to the system.
 * @ingroup stariface 
 * 
 * An example of this plugin is the one that fills the "file" menu and its Save/Load entries.
 * They are used to modularize the structure of the GUI system.
 */
class STARLIB_EXPORT GuiPlugin : public StarlabPlugin{
public:
    /// Loads functionalities (menu entries and their logic) into the StarlabMainWindow
    virtual void load() = 0;
    /// If Gui elements need to be updated upon change (i.e. disabled, removed), then overlaod this method.
    virtual void update(){}

protected:
    Starlab::Model* selectedModel(){ return document()->selectedModel(); }
    using StarlabPlugin::application;
    using StarlabPlugin::mainWindow;
    using StarlabPlugin::settings;
    using StarlabPlugin::pluginManager;
    using StarlabPlugin::document;
    using StarlabPlugin::drawArea; 
};

Q_DECLARE_INTERFACE(GuiPlugin, "Starlab::GuiPlugin/2.0")
