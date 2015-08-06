#pragma once
#include "interfaces/GuiPlugin.h"
#include "AboutPluginsDialog.h"
#include "StarlabMainWindow.h"

// #include "ui_aboutDialog.h"

#ifdef DEBUG_PLUGIN_HELPMENU
    #define DEB 
#else
    #define DEB if(false)
#endif

class plugin_gui_helpmenu : public GuiPlugin{
    Q_OBJECT
    Q_INTERFACES(StarlabPlugin)
    Q_INTERFACES(GuiPlugin)
    
private:
    StarlabMainWindow* window;

public:
    void load(StarlabMainWindow* window){
        this->window = window;
        
        if(false){ /// @todo implement
            QAction* aboutAct = new QAction(tr("&About"), this);
            connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));
            window->helpMenu->addAction(aboutAct);            
        }
        if(false){ /// @todo implement
            QAction* aboutPluginsAct = new QAction(tr("About &Plugins"), this);
            connect(aboutPluginsAct, SIGNAL(triggered()), this, SLOT(aboutPlugins()));
            window->helpMenu->addAction(aboutPluginsAct);
        }
        if(false){ /// @todo implement
            QAction* onlineHelpAct = new QAction(tr("Online &Documentation"), this);
            connect(onlineHelpAct, SIGNAL(triggered()), this, SLOT(helpOnline()));
            window->helpMenu->addAction(onlineHelpAct);
        }
        if(false){ /// @todo implement
            QAction* submitBugAct = new QAction(tr("Submit Bug"), this);
            connect(submitBugAct, SIGNAL(triggered()), this, SLOT(submitBug()));
            window->helpMenu->addAction(submitBugAct);
        }
        if(false){ /// @todo implement
            QAction* onscreenHelpAct = new QAction(tr("On screen quick help"), this);
            connect(onscreenHelpAct, SIGNAL(triggered()), this, SLOT(helpOnscreen()));
            window->helpMenu->addAction(onscreenHelpAct);
        }
        if(false){ /// @todo implement
            QAction* checkUpdatesAct = new QAction(tr("Check for updates"), this);
            connect(checkUpdatesAct, SIGNAL(triggered()), this, SLOT(checkForUpdates()));
            window->helpMenu->addAction(checkUpdatesAct);            
        }
    }
};
