#include "gui_render.h"
Q_EXPORT_PLUGIN(gui_render)

#include "StarlabDrawArea.h"
#include <QDockWidget>
#include <QColorDialog>

using namespace Starlab;

/// Since we depend on the selected model, the load is minimal
void gui_render::load(){
    // qDebug() << "gui_render::load()";
    renderModeGroup = new QActionGroup(this);
    renderModeGroup->setExclusive(true);
    currentAsDefault = new QAction("Set current as default...",this);
    qColorDialog = NULL;    

    editRenderSettings = new QAction("Edit renderer settings...",this);
    editModelColor = new QAction("Change model color...",this);
    editBackgroundColor = new QAction("Change background color...",this);
    clearRenderObjects = new QAction (tr("Clear render objects"), this);
    clearRenderObjects->setToolTip("Removes all render objects from the scene. Render objects" \
                       "are typically used to visually debug the algorithms. "\
                       "This function allows you to clear them from the scene.");
    
    /// When document changes, we make sure render menu/toolbars are up to date    
    connect(document(), SIGNAL(hasChanged()), this, SLOT(update()));
}

void gui_render::update(){
    // qDebug() << "gui_render::update()";
    toolbar()->clear();
    menu()->clear();

    /// Fetch current renderer
    Model* selectedModel = document()->selectedModel();
    if(selectedModel==NULL){
        toolbar()->setVisible(false);
        return;
    }
    
    Renderer* currentRenderer = selectedModel->renderer();
    
    /// Add render modes menu/toolbar entries
    foreach(RenderPlugin* plugin, pluginManager()->getApplicableRenderPlugins(selectedModel)){
        QAction* action = plugin->action();
        action->setCheckable(true);
        
        /// "Check" an icon
        if(currentRenderer!=NULL)
            if(currentRenderer->plugin() == plugin)
                action->setChecked("true");
        
        renderModeGroup->addAction(action);
        /// If it has icon.. add it to toolbar
        if(!action->icon().isNull())
            toolbar()->addAction(action);
    }

    /// Make toolbar visible if there is something to show
    toolbar()->setVisible(toolbar()->actions().size()>0);
    
    /// @internal menu can be added only after it has been filled :(
    menu()->addAction(clearRenderObjects);
    menu()->addAction(editRenderSettings);
    menu()->addAction(currentAsDefault);
    menu()->addAction(editModelColor);
    menu()->addAction(editBackgroundColor);
    menu()->addSeparator();
    menu()->addActions(renderModeGroup->actions());
        
    /// Disable settings link when no parameters are given
    editRenderSettings->setDisabled(currentRenderer->parameters()->isEmpty());
    
    /// Connect click events to change in renderer system
    connect(renderModeGroup, SIGNAL(triggered(QAction*)), this, SLOT(triggerRenderModeAction(QAction*)), Qt::UniqueConnection);
    connect(currentAsDefault, SIGNAL(triggered()), this, SLOT(triggerSetDefaultRenderer()), Qt::UniqueConnection);
    connect(editRenderSettings, SIGNAL(triggered()), this, SLOT(trigger_editSettings()),Qt::UniqueConnection);
    connect(editModelColor, SIGNAL(triggered()), this, SLOT(trigger_editSelectedModelColor()), Qt::UniqueConnection);
    connect(editBackgroundColor, SIGNAL(triggered()), this, SLOT(trigger_editBackgroundColor()), Qt::UniqueConnection);
    connect(clearRenderObjects, SIGNAL(triggered()), drawArea(), SLOT(clear()), Qt::UniqueConnection);    
}

void gui_render::triggerSetDefaultRenderer(){
    // qDebug() << "gui_render::triggerSetDefaultRenderer()";
    Model* model = document()->selectedModel();    
    RenderPlugin* renderer = model->renderer()->plugin();
    pluginManager()->setPreferredRenderer(model,renderer);
    QString message = QString("Preferred renderer for \"%1\" set to \"%2\"")
                              .arg(model->metaObject()->className())
                              .arg(renderer->name());
    mainWindow()->setStatusBarMessage(message);
}

void gui_render::trigger_editSettings(){
    Renderer* renderer = document()->selectedModel()->renderer();
    /// No renderer set (btw... weird) skip
    if(renderer==NULL) return;
    /// No parameters.. avoid useless empty widget
    if(renderer->parameters()->isEmpty()) return;
    
    /// Create 
    ParametersWidget* widget = new ParametersWidget(renderer->parameters(), mainWindow());
    
    /// Add a simple title
    widget->setWindowTitle(QString("Settings for '%1'").arg(renderer->plugin()->name()));
    
    /// Behave as independent window & stay on top
    widget->setWindowFlags(Qt::Window | Qt::WindowStaysOnTopHint);
    
    /// Delete frame when its associated renderer gets killed
    connect(renderer, SIGNAL(destroyed()), widget, SLOT(deleteLater()));
    
    /// On a change in parameters, re-init, render and update
    connect(widget, SIGNAL(parameterChanged()), renderer, SLOT(init()));
    connect(widget, SIGNAL(parameterChanged()), renderer, SLOT(render()));
    connect(widget, SIGNAL(parameterChanged()), drawArea(), SLOT(updateGL()));    
    
    /// Finally show
    widget->show(); 
}

void gui_render::instantiate_color_dialog(){
    /// @internal on mac the (pretty) native dialog is buggy... randomly the native one opens
    /// https://bugreports.qt-project.org/browse/QTBUG-11188  
    /// Disconnect object from previous connections
    if(qColorDialog != NULL){
        qColorDialog->disconnect();
        return;
    }
    
    qColorDialog = new QColorDialog();
    qColorDialog->hide();
    qColorDialog->setOption(QColorDialog::ShowAlphaChannel, true);
    qColorDialog->setOption(QColorDialog::DontUseNativeDialog,false);
    qColorDialog->setOption(QColorDialog::NoButtons,true);
    qColorDialog->setWindowFlags(Qt::X11BypassWindowManagerHint | Qt::WindowStaysOnTopHint);
    connect(mainWindow(), SIGNAL(destroyed()), qColorDialog, SLOT(deleteLater()));    
}

void gui_render::trigger_editBackgroundColor(){
    instantiate_color_dialog();
    qColorDialog->setCurrentColor( drawArea()->backgroundColor() );
    connect(qColorDialog, SIGNAL(currentColorChanged(QColor)), this, SLOT(liveupdate_backgroundColor(QColor)));
    // Predefind background colors
    qColorDialog->setCustomColor(0, QColor(255,255,255).rgb());
    qColorDialog->setCustomColor(1, QColor(208,212,240).rgb());
    qColorDialog->setCustomColor(2, QColor(50,50,60).rgb());
    qColorDialog->setCustomColor(3, QColor(0,0,0).rgb());
    qColorDialog->show();
}

void gui_render::trigger_editSelectedModelColor(){
    instantiate_color_dialog();
    connect(qColorDialog, SIGNAL(currentColorChanged(QColor)), this, SLOT(liveupdate_selectedModelColor(QColor)));
    qColorDialog->setCurrentColor(document()->selectedModel()->color);
    qColorDialog->show();
}

void gui_render::liveupdate_backgroundColor(QColor color){
    /// Force-remove background color from snapshots
    drawArea()->setBackgroundColor(color);
    drawArea()->updateGL();   
    
    /// Save it in the settings
    QString key = "DefaultBackgroundColor";
    settings()->set( key, QVariant( color ) );
    settings()->sync();
}

void gui_render::liveupdate_selectedModelColor(QColor color){
    Model* model = document()->selectedModel();
    if(model==NULL) return;
    model->color = color;
    drawArea()->updateGL();        
}


void gui_render::triggerRenderModeAction(QAction* action){
    // qDebug("gui_render::triggerRenderModeAction(\"%s\")",qPrintable(action->text()));
    Model* model = document()->selectedModel();
    RenderPlugin* plugin = pluginManager()->getRenderPlugin(action->text());
    model->setRenderer(plugin);
    drawArea()->updateGL();    
}

