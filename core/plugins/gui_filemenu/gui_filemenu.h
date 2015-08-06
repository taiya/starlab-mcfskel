#pragma once
#include <QToolBar>
#include <QFileDialog>
#include "interfaces/GuiPlugin.h"
#include "StarlabMainWindow.h"

class gui_filemenu : public GuiPlugin{
    Q_OBJECT
    Q_INTERFACES(GuiPlugin)
           
private slots:
    void open(); ///< Smart open (open project/files)
    void save(); ///< Smart save (save project/files)
    void reload_selection();
    
    void save_project(){ qWarning("not implemented"); }
    void save_project_as(){ qWarning("not implemented"); }
    void save_selection(){ qWarning("not implemented"); }
    void save_selection_as();
    void reload_project(){ qWarning("not implemented"); }
    void take_screenshot();
    void delete_selected_model();
    
public:    
    void load(){
        // bool docNotEmpty = !mainWindow()->document()->isEmpty();

        /// Opens a model/project
        { 
            QAction* action = new QAction(QIcon(":/images/import_mesh.png"),tr("Open (Project or Model)"), this);
            action->setShortcutContext(Qt::ApplicationShortcut);
            action->setShortcut(Qt::CTRL+Qt::Key_O); ///< @todo change to cross platform open
            connect(action, SIGNAL(triggered()), this, SLOT(open()));            
            mainWindow()->fileMenu->addAction(action);
            mainWindow()->mainToolbar->addAction(action);
        }     

        /// Smart save
        { 
            QAction* action = new QAction(QIcon(":/images/save.png"),tr("Save (Project or Model)"), this);
            action->setShortcutContext(Qt::ApplicationShortcut);
            action->setShortcut(Qt::CTRL+Qt::Key_S); ///< @todo change to cross platform save
            connect(action, SIGNAL(triggered()), this, SLOT(save()));
            mainWindow()->fileMenu->addAction(action);
            mainWindow()->mainToolbar->addAction(action);
        }
        
        /// @todo Save selection as...
        { 
            QAction* action = new QAction(QIcon(":/images/save.png"),tr("Save Selection as..."), this);
            connect(action, SIGNAL(triggered()), this, SLOT(save_selection_as()));
            mainWindow()->fileMenu->addAction(action);
        }
        
        /// Delete selected model
        {
            QAction* action = new QAction("Delete selected model", this);
            action->setShortcutContext(Qt::ApplicationShortcut);
            connect(action, SIGNAL(triggered()), this, SLOT(delete_selected_model()));
            mainWindow()->fileMenu->addAction(action);
        }
        
        /// Takes a qglViewer screenshot
        { 
            QAction* action = new QAction(QIcon(":/images/screenshot.png"),tr("Take Screenshot"), this);
            connect(action, SIGNAL(triggered()), this, SLOT(take_screenshot()));
            mainWindow()->fileMenu->addAction(action);
            mainWindow()->mainToolbar->addAction(action);
        }
        
        /// @todo Saves a project (and recursively save all models under)
        if(false){ 
            QAction* action = new QAction(QIcon(":/images/save.png"),tr("Save Project"), this);
            connect(action, SIGNAL(triggered()), this, SLOT(save_project()));
            mainWindow()->fileMenu->addAction(action);
        }
        
        /// @todo Saves a project as...
        if(false){ 
            QAction* action = new QAction(QIcon(":/images/save.png"),tr("Save Project As.."), this);
            connect(action, SIGNAL(triggered()), this, SLOT(save_project_as()));
            mainWindow()->fileMenu->addAction(action);
        }
        
        /// @todo Save selection
        if(false){ 
            QAction* action = new QAction(QIcon(":/images/save.png"),tr("Save Selection"), this);
            connect(action, SIGNAL(triggered()), this, SLOT(save_selection()));
            mainWindow()->fileMenu->addAction(action);
        }
        
        mainWindow()->fileMenu->addSeparator();
        
        /// @todo Revert project (and models recursively)
        if(false){
            QAction* action = new QAction(QIcon(":/images/reload.png"),tr("Revert Project"), this);
            connect(action, SIGNAL(triggered()), this, SLOT(reload_project()));
            mainWindow()->fileMenu->addAction(action);
        }
        
        /// Revert (selection)
        if(true){
            QAction* action = new QAction(QIcon(":/images/reload.png"),tr("Revert Selection"), this);
            action->setShortcutContext(Qt::ApplicationShortcut);
            action->setShortcut(Qt::CTRL+Qt::Key_R);
            connect(action, SIGNAL(triggered()), this, SLOT(reload_selection()));
            mainWindow()->fileMenu->addAction(action);
            mainWindow()->mainToolbar->addAction(action);
        }
        
        /// Quit
        {
            QAction* action = new QAction(tr("E&xit"), this);
            action->setShortcut(Qt::CTRL+Qt::Key_Q);
            connect(action, SIGNAL(triggered()), mainWindow(), SLOT(close()));
            mainWindow()->fileMenu->addAction(action);
        }
    }
};
