#include "StarlabApplication.h"
#include "FilterPlugin.h"
#include "parameters/RichParameterSet.h"

using namespace Starlab;

Application::Application(){
    /// Instantiate resources
    _settings      = new Settings(this);
    _pluginManager = new PluginManager(_settings);
    _document      = new Document();
    
    /// Register all plugins access functionality
    foreach(StarlabPlugin* plugin, pluginManager()->plugins())
        plugin->_application = this;
    
    /// Register Starlab.h types
    qRegisterMetaType<BBox3>("BBox3");
}

Application::~Application(){
    delete _pluginManager;
    delete _document;
    delete _settings;
}

bool Application::saveModel(Model* model, QString path){
    if(path.isEmpty()) path=model->path;
    // qDebug() << "Saving model: " << model->name << " to file: " << path;
    QFileInfo fileInfo(path);
    
    /// Extract or set default extension
    QString extension = fileInfo.suffix().toLower();
    // qDebug() << "requested extension: " << extension;
    if( extension.isEmpty() ) extension = "off";        
    
    /// Checks a suitable plugin exists
    InputOutputPlugin* iIO = pluginManager()->modelExtensionToPlugin[extension];
    if( !iIO ) throw StarlabException("Cannot find plugin suitable for the provided extension %s", qPrintable(extension));
    iIO->save(model,path);            
    
    return true;
}

bool Application::loadModel(QString path, InputOutputPlugin* plugin){
    // qDebug("StarlabApplication::loadModel(\"%s\")", qPrintable(path));    
    QFileInfo fileInfo(path);
    QString extension = fileInfo.suffix().toLower();
    QString basename = fileInfo.completeBaseName();
    
    if(plugin==NULL){
        plugin = pluginManager()->modelExtensionToPlugin[extension];
        if(plugin==NULL) return false;
    }
    
    /// Checks a suitable plugin exists
    InputOutputPlugin* iIO = pluginManager()->modelExtensionToPlugin[extension];
    if(iIO == NULL) throw StarlabException("File '%s' has not been opened becase format '%s' not supported", qPrintable(basename), qPrintable(extension));
    
    /// Checks file existence
    iIO->checkReadable(path);
    
    /// Calls the plugin for open operation
    document()->pushBusy();
        Model* newModel = iIO->open(path);
        if(newModel==NULL) throw StarlabException("Attempted to create a NULL model");
        /// Update the bounding box (for rendering reasons)
        /// @todo Would it be possible to Qt-Connect iIO->open to updateBoundingBox()?
        newModel->updateBoundingBox();
        document()->addModel( newModel ); 
    document()->popBusy();

    /// Set as selected model
    document()->setSelectedModel( newModel );

    document()->emit_resetViewport();
    
    return true;
}
bool Application::loadProject(QString path, ProjectInputOutputPlugin* plugin){   
    // qDebug("StarlabApplication::loadProject(\"%s\")", qPrintable(path));
    QFileInfo fileInfo(path);
    QString extension = fileInfo.suffix().toLower();
    QString basename = fileInfo.completeBaseName();    

    if(plugin==NULL){
        plugin = pluginManager()->projectExtensionToPlugin[extension];
        if(plugin==NULL) 
            return false;
    }
    
    /// Checks a suitable plugin exists   
    ProjectInputOutputPlugin* iIO = pluginManager()->projectExtensionToPlugin[extension];
    
    /// Checks file existence
    if(iIO == NULL)            throw StarlabException("Project file '%s' has not been opened, format %s not supported", qPrintable(basename), qPrintable(extension));
    if(!fileInfo.exists())     throw StarlabException("Project file '%s' does not exist", qPrintable(path));
    if(!fileInfo.isReadable()) throw StarlabException("Project file '%s' is not readable", qPrintable(path));
    
    /// Clear the doc and use plugin to fill in
    document()->pushBusy();
        document()->clear();
        iIO->open(path, *this);
    document()->popBusy();
    
    /// Refresh visualization
    document()->emit_resetViewport();
    
    /// @todo catch exceptions in iIO->open and return false on any caught exception
    return true;
}

QList<FilterPlugin*> Application::applicableFilters(){
    return pluginManager()->filterPlugins();
}

QList<FilterPlugin *> Application::applicableFilters(Model *model){
    QList<FilterPlugin*> retval;    
    foreach(FilterPlugin* plugin, pluginManager()->filterPlugins())
        if(plugin->isApplicable(model))
            retval.append(plugin);
    return retval;
}

void Application::load(QString path){
    // qDebug("StarlabApplication::load(\"%s\")", qPrintable(path));
    bool retstatus = false;
    if(!retstatus) retstatus = loadModel(path,NULL);
    if(!retstatus) retstatus = loadProject(path,NULL);
    
    /// Nothing was able to open
    if(!retstatus)
        throw StarlabException("Starlab does not know how to open file: " + path);
}

void Application::executeFilter(Model* model, QString filterName){
    // qDebug() << "StarlabApplication::executeFilter()";
    FilterPlugin* filter = pluginManager()->getFilter(filterName);
    if(!filter->isApplicable(model)) 
        throw StarlabException("Filter not applicable");
    
    /// Filter is applied on the *selected* model
    document()->setSelectedModel(model);
    RichParameterSet* pars = new RichParameterSet();
    filter->initParameters(pars);
    filter->applyFilter(pars);
    pars->deleteLater();
}

QDir Application::starlabDirectory(){
    QDir baseDir(QApplication::applicationDirPath());
    if( OSQuery::isMac() ){            
        baseDir.cdUp();
        baseDir.cdUp();
        return baseDir.absolutePath();
    }
    if( OSQuery::isWin() )
        return QCoreApplication::applicationDirPath();
    if( OSQuery::isLinux() )
        throw StarlabException("TODO: FIX THE INI LOAD PATH FOR LINUX!!!");

    /// @todo is there better way of doing this without having to return dummy?
    Q_ASSERT(false);
    return QDir("");
}

QDir Application::executionDirectory(){
    return QFileInfo("./").absoluteDir();
}

