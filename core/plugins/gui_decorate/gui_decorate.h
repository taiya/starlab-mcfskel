#pragma once
#include <QToolBar>

#include "StarlabMainWindow.h"
#include "interfaces/GuiPlugin.h"
#include "interfaces/RenderPlugin.h"
#include "interfaces/DecoratePlugin.h"

class gui_decorate : public GuiPlugin{
    Q_OBJECT
    Q_INTERFACES(GuiPlugin)
    
    /// @{ Load/Update
    public:    
        void load();    
        void update();
    private:
        QActionGroup* decoratorGroup;
    /// @}
    
    /// @{ Decorate Logic
    private:    
        QToolBar* toolbar(){ return mainWindow()->decorateToolbar; }
        QMenu* menu(){ return mainWindow()->decorateMenu; }
    public slots:
        void toggleDecorator(QAction* );
    /// @}
};
