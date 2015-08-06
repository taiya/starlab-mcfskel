#include <QPluginLoader>

#include "StarlabPlugin.h"
#include "StarlabMainWindow.h"

using namespace Starlab;

StarlabPlugin::StarlabPlugin(){ 
    _mainWindow = NULL;
    _application = NULL;
    _action = NULL; 
}

DrawArea* StarlabPlugin::drawArea(){ 
    if(_mainWindow==NULL) return NULL;
    return mainWindow()->drawArea(); 
}

MainWindow* StarlabPlugin::mainWindow(){ 
	return _mainWindow; 
}
 
Application* StarlabPlugin::application(){ 
	return _application; 
}

Document* StarlabPlugin::document(){ 
	Q_ASSERT(_application);
	return _application->document(); 
}

Settings* StarlabPlugin::settings(){ 
	Q_ASSERT(_application);
	return _application->settings(); 
}

PluginManager* StarlabPlugin::pluginManager(){ 
	Q_ASSERT(_application);
	return _application->pluginManager();
}

/// @internal action cannot be created in constructor. This is because 
/// in the constructor local methods are preferred over polymorphic ones :(
QAction *StarlabPlugin::action(){ 
    if(_action==NULL){
        _action = new QAction(icon(),name(),NULL);
        _action->setToolTip(description());
        _action->setShortcut(shortcut());
        _action->setParent(this);        
    }
    return _action;
}

void StarlabPlugin::showMessage(const char *format, ...){
    const size_t buffer_length=256;
    char buffer[buffer_length];
    va_list args;
    va_start (args, format);
    vsnprintf(buffer,buffer_length,format, args);
    va_end (args);
    QString msg(buffer);
    /// Show on terminal
    qDebug() << msg;
    /// And on window
    if( mainWindow() )
        mainWindow()->setStatusBarMessage(msg,20.0);
}

void StarlabPlugin::showProgress(double val){
    if( mainWindow() ){
        mainWindow()->setStatusBarMessage( "Executing: '" + this->name() + "'..." );
        mainWindow()->setProgressBarValue(val);
    }
}
