#pragma once
#include "interfaces/GuiPlugin.h"
#include "interfaces/ModePlugin.h"

class StateMachine;

/// At any point there can be a single editing plugin active.
/// When a plugin is active (i.e. not suspended) it intercept 
/// all the input (i.e. mouse/keyboard) actions.
class gui_mode : public GuiPlugin{
    Q_OBJECT
    Q_INTERFACES(GuiPlugin)

/// The gui entry that suspends a plugin
public:
    QAction* defaultModeAction;    
    QActionGroup* modeActionGroup;
        
/// plugin constructor
void load();
public slots:
    void update();

/// @{ State machine to manage suspension
private:
    /// The possible states of the machine
    enum STATE {DEFAULT, SUSPENDED, MODE};
    /// The current machinse state
    STATE state;
    /// The action that was suspended   
    QAction* lastActiveModeAction;
    /// Called when entering a state
    void enterState(STATE state, QAction* action=NULL);
public slots:
    /// This causes changes of states
    void actionClicked(QAction *action);
/// @}
    
public slots:   
    /// Responds to a changes in document. If the plugin specifies its own way 
    /// to respond to the event, this is used. This can be done by overloading 
    /// ModePlugin::documenChanged(). If no custom behavior is provided, we simply 
    /// call ModePlugin::destroy(), ModePlugin::create() in succession.
    void documentChanged();
};
