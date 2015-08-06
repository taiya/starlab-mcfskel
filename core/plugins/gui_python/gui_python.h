#pragma once
#include <QToolBar>
#include <QDockWidget>

#include "StarlabMainWindow.h"
#include "GuiPlugin.h"

class gui_python : public GuiPlugin{
    Q_OBJECT
    Q_INTERFACES(GuiPlugin)

private:
    QDockWidget* widget;
    
/// @{ Load/Update
public:    
    void load();    
    void update(){}
/// @}

/// @{ Slots
public slots:
    void modePluginSetSlot(ModePlugin*);
/// @}
    
/// @{ Quick hooks Logic
private:    
    QToolBar* toolbar(){ return mainWindow()->mainToolbar; }
    QMenu* menu(){ return mainWindow()->windowsMenu; }
/// @}

};
