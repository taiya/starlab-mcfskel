#pragma once
#include <QMenu>
#include "interfaces/GuiPlugin.h"
#include "layerDialog.h"

#ifdef DEBUG_PLUGIN_VIEWCONTROL
    #define DEB 
#else
    #define DEB if(false)
#endif

class gui_windows : public GuiPlugin{
    Q_OBJECT
    Q_INTERFACES(GuiPlugin)
    
private:
    LayerDialog* layerDialog;
    
public:
    void load(){
        /// Sets up the layer dialog, always allocated (usually is just hidden)
        {
            layerDialog = new LayerDialog(mainWindow());
            layerDialog->setAllowedAreas (Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
            mainWindow()->addDockWidget(Qt::RightDockWidgetArea,layerDialog);
            QAction* action = new QAction(QIcon(":/images/layers.png"),tr("Show Layers Dialog"), this);
            action->setCheckable(true);
            action->setChecked(false);
            mainWindow()->mainToolbar->addAction(action);
            mainWindow()->windowsMenu->addAction(action);
            connect(action, SIGNAL(triggered(bool)), layerDialog, SLOT(setVisible(bool)));
        }
        
        /// Snapshot setup
        {
            QAction* action = new QAction(QIcon(":/images/snapshot.png"),tr("Save snapsho&t"), this);
            // connect(action, SIGNAL(triggered()), this, SLOT(saveSnapshot()));
            action->setEnabled(false);
        }
    }
        
    virtual void update(){
        if(layerDialog->isVisible())
            layerDialog->updateTable();
    }
};
