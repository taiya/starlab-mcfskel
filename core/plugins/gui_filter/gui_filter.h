#pragma once
#include "interfaces/GuiPlugin.h"
#include "StarlabMainWindow.h"

/// At any point there can be a single editing plugin active.
/// When a plugin is active (i.e. not suspended) it intercept 
/// all the input (i.e. mouse/keyboard) actions.
class gui_filter : public GuiPlugin{
    Q_OBJECT
    Q_INTERFACES(GuiPlugin)
    
public:    
    /// plugin constructor
    virtual void load();
    
private slots:
    /// Opens up the gui where the user inputs/modifies the parameters and buttons to start the filter.   
    void startFilter();
    /// callback function that starts the filter, it is called by the 
    /// filter popup when the user presses "ok" or by a command line instruction
    void execute(FilterPlugin* iFilter, RichParameterSet *pars);
};
