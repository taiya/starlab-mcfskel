#include "PluginManager.h"
#include <QObject>
#include <QRegExp>
#include <QPluginLoader>

#include "StarlabException.h"
#include "StarlabSettings.h"
#include "RenderPlugin.h"
#include "DecoratePlugin.h"
#include "FilterPlugin.h"
#include "GuiPlugin.h"
#include "ModePlugin.h"

#if defined(Q_OS_WIN)
    QString LIB_EXTENSION_FILTER("*.dll");
#elif defined(Q_OS_MAC)
    QString LIB_EXTENSION_FILTER("*.dylib");
#elif defined(Q_OS_LINUX)
    QString LIB_EXTENSION_FILTER("*.so");
#endif

using namespace Starlab;
    
/// Input: a string like "[SurfaceMesh] Object File Format (*.off,*.off2)"
/// Returns a lowercase stringlist: {"off","off2"}
QStringList extractExtensions(QString iopluginname){
    QStringList retval;
    
    // qDebug() << "Parsing string: " << iopluginname;
    int i_open  = iopluginname.indexOf("(");
    int i_close = iopluginname.indexOf(")");
    
    // Chunk of string between parenthesis, then split by comma
    iopluginname = iopluginname.mid(i_open+1,i_close-i_open-1);
    QStringList extensions = iopluginname.split(QRegExp(","));
    foreach(QString extension, extensions){
        int i_dot = extension.indexOf(".");
        extension = extension.mid(i_dot+1);
        // qDebug() << "extracted extension: " << extension;
        retval.append( extension.toLower() );
    }
    
    return retval;
}
    
QString failurecauses_qtplugin(
"\nPOSSIBLE FAILURE REASONS:\n"
"  1) plugin needs a DLL which cannot be found in the executable folder\n"
"  2) Release / debug build mismatch\n"
"  3) Missing Q_INTERFACE(...) or Q_EXPORT_PLUGIN(...)\n"
"  *) any other reason?");

QString failurecauses_starlabplugin(
"\nPOSSIBLE FAILURE REASONS:\n"
"  1) starlab PluginManager does not know how to load it\n"
"  2) Release / debug build mismatch.\n"
"  *) any other reason?");

PluginManager::PluginManager(Settings* settings) : 
    _settings(settings)
{
    //pluginsDir=QDir(getPluginDirPath()); 
    // without adding the correct library path in the mac the loading of jpg (done via qt plugins) fails
    //qApp->addLibraryPath(getPluginDirPath());
    //qApp->addLibraryPath(getBaseDirPath());
    
    try 
    {
        loadPlugins();
    } 
    STARLAB_CATCH_BLOCK 
}

FilterPlugin *PluginManager::getFilter(QString name){
    FilterPlugin* filter = _filterPlugins.value(name,NULL);
    if(filter==NULL) throw StarlabException("Cannot find filter '%s'", qPrintable(name));
    return filter;
}

void PluginManager::loadPlugins() {
    // qDebug() << "PluginManager::loadPlugins(..)";
    
    pluginsDir=QDir(getPluginDirPath());
    // without adding the correct library path in the mac the loading of jpg (done via qt plugins) fails
    // qApp->addLibraryPath(getPluginDirPath());
    // qApp->addLibraryPath(getBaseDirPath());
    QStringList pluginfilters(LIB_EXTENSION_FILTER);
    //only the files with appropriate extensions will be listed by function entryList()
    pluginsDir.setNameFilters(pluginfilters);

    // qDebug( "Loading plugins from: %s ",qPrintable(pluginsDir.absolutePath()));
    
    /// Load all plugins
    foreach (QString fileName, pluginsDir.entryList(QDir::Files)) {
        
        /// Attempt to load a Qt plugin
        QString path = pluginsDir.absoluteFilePath(fileName);
        QPluginLoader loader(path);
        QObject* plugin = loader.instance();
        if(!plugin){
            qDebug("Plugin '%s' is not a proper *Qt* plugin!! %s", qPrintable(fileName), qPrintable(failurecauses_qtplugin));
            continue;
        }
            
        /// Attempt to load one of the starlab plugins
        bool loadok = false;
        loadok |= load_InputOutputPlugin(plugin);
        loadok |= load_ProjectInputOutputPlugin(plugin);
        loadok |= load_FilterPlugin(plugin);       
        loadok |= load_DecoratePlugin(plugin);
        loadok |= load_GuiPlugin(plugin);        
        loadok |= load_EditPlugin(plugin);
        loadok |= load_RenderPlugin(plugin);
        if( !loadok ) 
            throw StarlabException("plugin "+fileName+" was not recognized as one of the declared Starlab plugin!!"); // +failurecauses_starlabplugin));
        
        StarlabPlugin* splugin = dynamic_cast<StarlabPlugin*>(plugin);
        if(!splugin) continue;
            
        /// Store pointers to all plugin
        _plugins.insert(splugin->name(),splugin);
        
        /// If we read here loading went ok
        // qDebug() << "Plugin: " << fileName << " loaded succesfully";
    }
}

QString PluginManager::getBaseDirPath(){
    QDir baseDir(qApp->applicationDirPath());

#if defined(Q_OS_WIN)
    // Windows:
    // during development with visual studio binary could be in the debug/release subdir.
    // once deployed plugins dir is in the application directory, so
    if (baseDir.dirName() == "debug" || baseDir.dirName() == "release")		
        baseDir.cdUp();
#endif

#if defined(Q_OS_MAC)
    /// In MAC application is always deployed as "Starlab.app/Contents/MacOS/starlab"
    /// So going 2 folders up we will find the plugin folder
    baseDir.cdUp();
    baseDir.cdUp();
#endif

    return baseDir.absolutePath();
}

QString PluginManager::getPluginDirPath() {
    QDir pluginsDir(getBaseDirPath());
    if(!pluginsDir.exists("plugins"))
        throw StarlabException("Serious error. Unable to find the plugins directory at %s.", qPrintable(pluginsDir.absoluteFilePath("plugins")) );
    pluginsDir.cd("plugins");
    return pluginsDir.absolutePath();
}

bool PluginManager::load_ProjectInputOutputPlugin(QObject *plugin){
    ProjectInputOutputPlugin* iIO = qobject_cast<ProjectInputOutputPlugin*>(plugin);
    if(!iIO) return false;
    projectIOPlugins.push_back(iIO);
    
    QStringList exts = extractExtensions( iIO->name() );
    foreach(QString ext, exts)
        projectExtensionToPlugin.insert(ext,iIO);

    return true;
}

bool PluginManager::load_InputOutputPlugin(QObject *plugin){
    InputOutputPlugin* iIO = qobject_cast<InputOutputPlugin*>(plugin);
    if(!iIO) return false;
    modelIOPlugins.insert(iIO->name(), iIO);

    /// Parse the extension filter into extensions
    QStringList exts = extractExtensions( iIO->name() );
    foreach(QString ext, exts)
        modelExtensionToPlugin.insert(ext,iIO);
    
    return true;
}

bool PluginManager::load_FilterPlugin(QObject *plugin){
    FilterPlugin* iFilter = qobject_cast<FilterPlugin*>(plugin);
    if(!iFilter) return false;
    _filterPlugins.insert(iFilter->name(), iFilter);
    return true;
}

bool PluginManager::load_DecoratePlugin(QObject *plugin){
    DecoratePlugin* iDecorate = qobject_cast<DecoratePlugin*>(plugin);
    if(!iDecorate) return false;
    _decoratePlugins.insert(iDecorate->name(), iDecorate);
    return true;
}
bool PluginManager::load_GuiPlugin(QObject* _plugin){
    GuiPlugin* plugin = qobject_cast<GuiPlugin*>(_plugin);
    if(!plugin) return false;
    _guiPlugins.insert(plugin->name(), plugin);
    return true;
}

bool PluginManager::load_EditPlugin(QObject* _plugin){
    ModePlugin* plugin = qobject_cast<ModePlugin*>(_plugin);
    if(!plugin) return false;
    _modePlugins.insert(plugin->name(), plugin);    
    return true;
}
 
bool PluginManager::load_RenderPlugin(QObject *_plugin){
    RenderPlugin* plugin = qobject_cast<RenderPlugin*>(_plugin);
    if(!plugin) return false;
    _renderPlugins.insert(plugin->name(), plugin);
    return true;
} 

QString PluginManager::getFilterStrings(){
    QStringList filters;
    
    /// Fill in filters for Model files
    /// @todo add the readable format name
    QStringList extensions= modelExtensionToPlugin.keys();
    foreach(QString extension, extensions)
        filters.append("*."+extension);
    
    /// Fill in filters for Document files
    filters.prepend("Starlab Project (*.starlab)");
    filters.prepend("All Files (*.*)");
    
    /// Merge in a ;; separated list
    return filters.join(";;");
}

RenderPlugin *PluginManager::getRenderPlugin(QString pluginName){
    RenderPlugin *plugin = _renderPlugins.value(pluginName,NULL);
    if(plugin==NULL) throw StarlabException("Renderer %s could not be found",qPrintable(pluginName));
    return plugin;
}

/// @todo rename getDecoratePlugin(QString name)
DecoratePlugin* PluginManager::newDecoratePlugin(QString /*pluginName*/, Model* /*model*/){
    throw StarlabException("Decoration needs refactoring (See Render)");
#if 0
    Q_ASSERT(model!=NULL);
    DecoratePlugin *plugin = _decoratePlugins.value(pluginName,NULL);
    if(plugin==NULL) 
        throw StarlabException("Could not find plugin '%s'.",qPrintable(pluginName));  
    DecoratePlugin* newplugin = plugin->factory();
    newplugin->_mainWindow = plugin->_mainWindow;
    newplugin->_application = plugin->_application;
    model->addDecoratePlugin(newplugin);
    newplugin->_action = plugin->action();
    return newplugin;
#endif
}

QString PluginManager::getPreferredRenderer(Model *model){
    QString key = "DefaultRenderer/"+QString(model->metaObject()->className());
    QString rendererName;
    if(settings()->contains(key)) 
        rendererName=settings()->getString(key);

    /// Preferred plugins could not be found
    if(!_renderPlugins.contains(rendererName)){
        foreach(RenderPlugin* plugin, _renderPlugins){
            if(plugin->isApplicable(model) && plugin->isDefault()){
                rendererName = plugin->name();
                break;
            }
        }
    }
    
    /// Couldn't find one that was marked as isDefault()
    /// just take the first that is applicable. 
    /// BBOX renderer should be found here!
    if(!_renderPlugins.contains(rendererName)){
        foreach(RenderPlugin* plugin, _renderPlugins){
            if(plugin->isApplicable(model)){
                rendererName = plugin->name();
                break;
            }
        }
    }

    /// Everything failed..  let's give up
    if(!_renderPlugins.contains(rendererName))
        throw StarlabException("No suitable render plugin found\nIs it possible you didn't compile the BBOX renderer?");

    return rendererName;
}

void PluginManager::setPreferredRenderer(Model *model, RenderPlugin* plugin){
    // qDebug("PluginManager::setPreferredRenderer(..,%s)",qPrintable(plugin->name()));
    Q_ASSERT(plugin!=NULL);
    QString key = "DefaultRenderer/"+QString(model->metaObject()->className());
    settings()->set(key,plugin->name());
    settings()->sync();
}

QList<RenderPlugin *> PluginManager::getApplicableRenderPlugins(Model* model){
    QList<RenderPlugin*> retval;
    Q_ASSERT(model!=NULL);
    foreach(RenderPlugin* plugin, _renderPlugins.values())
        if( plugin->isApplicable(model) )
            retval.append(plugin);
    return retval;
}
