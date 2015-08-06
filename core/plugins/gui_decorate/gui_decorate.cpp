#include "gui_decorate.h"
#include "StarlabDrawArea.h"

using namespace Starlab;

/// Since we depend on the selected model, the load is minimal
void gui_decorate::load(){
    // qDebug() << "gui_decorate::load()";
    this->decoratorGroup = new QActionGroup(this);
    this->decoratorGroup->setExclusive(false);
}

void gui_decorate::update(){
    toolbar()->clear();
    menu()->clear();
   
    /// Fills the toolbar with applicable decorators
    foreach(DecoratePlugin* plugin, pluginManager()->decoratePlugins()){
        if(!plugin->isApplicable(document()->selectedModel())) 
            continue;
        QAction* action = plugin->action();
        action->setCheckable(true);
        /// They will be checked if active below
        action->setChecked(false);
        /// Add to the group (for easy event mngmnt)
        decoratorGroup->addAction( action );
        /// Insert in menus/toolbars
        menu()->insertAction(NULL,action);
        if(!action->icon().isNull())
            toolbar()->insertAction(NULL,action);
    }

    /// Mark active decorators as checked    
    foreach(DecoratePlugin* decorator, document()->selectedModel()->decoratePlugins())
        decorator->action()->setEnabled(true);        
    
    /// Connect it...
    connect(decoratorGroup, SIGNAL(triggered(QAction*)), this, SLOT(toggleDecorator(QAction*)), Qt::UniqueConnection);
}

void gui_decorate::toggleDecorator(QAction* action){
    qDebug() << "gui_decorate::toggleDecorator()" << action->isChecked();
    
    DecoratePlugin* plugin = qobject_cast<DecoratePlugin*>(action->parent());
    Q_ASSERT(plugin);
    Model* model = document()->selectedModel();
    if(action->isChecked()){
        Q_ASSERT(!model->hasDecoratePlugin(plugin));
        model->addDecoratePlugin(plugin);
    }
    else{
        Q_ASSERT(model->hasDecoratePlugin(plugin));
        model->removeDecoratePlugin(plugin);
    }

    /// Update visualization
    drawArea()->updateGL(); /// Refresh the window
}

Q_EXPORT_PLUGIN(gui_decorate)
