#pragma once
#include <QMap>
#include <QObject>
#include <QDir>
#include "starlib_global.h"

/// @{ forward declarations
    class DrawAreaPlugin;
    class RenderPlugin;
    class DecoratePlugin;
    class FilterPlugin;
    class GuiPlugin;
    class ModePlugin;
    /// @todo why am I not allowed to forward declare these?
    #include "interfaces/InputOutputPlugin.h"
    #include "interfaces/ProjectInputOutputPlugin.h"
/// @}
    
namespace Starlab{

/// \brief This class provides the basic tools for managing all the plugins. It is used by both meshlab and meshlab server.
class STARLIB_EXPORT PluginManager: public QObject{
    Q_OBJECT
    
/// @{ constructor
public:
    PluginManager(Settings *settings);
private:
    Settings* _settings;
    Settings* settings(){ return _settings; }
/// @}
    
/// @{ 
private:
    /// @brief Stores all (loaded) plugins
    QMap<QString,StarlabPlugin*>    _plugins;
    QMap<QString,GuiPlugin*>        _guiPlugins;
    QMap<QString,FilterPlugin*>     _filterPlugins;
    QMap<QString,ModePlugin*>       _modePlugins;
    QMap<QString,DecoratePlugin*>   _decoratePlugins;
    QMap<QString,RenderPlugin*>     _renderPlugins;    
public:
    /// @brief pointers to all the loaded plugins
    QList<StarlabPlugin*> plugins(){ return _plugins.values(); }
    /// @brief pointers to plugins subset
    QList<FilterPlugin*> filterPlugins(){ return _filterPlugins.values(); }
    QList<DecoratePlugin*> decoratePlugins(){ return _decoratePlugins.values(); }
    QList<ModePlugin*> modePlugins(){ return _modePlugins.values(); }
    QList<GuiPlugin*> guiPlugins(){ return _guiPlugins.values(); }
public:
    /// @brief pointer to specific plugin
    FilterPlugin* getFilter(QString name);
/// @}
        
public:  
    /// Stores the loaded IO model plugins
    /// @todo necessary? just use the values of the hash map 
    QMap<QString,InputOutputPlugin*> modelIOPlugins;
    /// Extension (lowercase) => IO model plugin
    QHash<QString,InputOutputPlugin*> modelExtensionToPlugin;
    
    /// Stores the loaded IO project plugins
    /// @todo necessary? just use the values of the hash map 
    QVector<ProjectInputOutputPlugin*> projectIOPlugins;   
    /// Extension (lowercase) => IO project plugin
    QHash<QString,ProjectInputOutputPlugin*> projectExtensionToPlugin;




/// @{ Render Plugins Control
public:
    /// Creates a new instance of a render plugin
    /// @exception if plugin with that name cannot be found
    RenderPlugin* getRenderPlugin(QString pluginName);
    QString getPreferredRenderer(Model* model);
    void setPreferredRenderer(Model* model, RenderPlugin* pluginName);
    
    /// Get the list of actions corresponding to render plugin that apply to this type of model
    QList<RenderPlugin*> getApplicableRenderPlugins(Model *model);
/// @}

/// @{ Decorate plugin control
    DecoratePlugin *newDecoratePlugin(QString pluginName, Model *model);
/// @}
        
public:
    /// Directory where plugins are found
    QDir pluginsDir;    
    static QString getPluginDirPath();
    
    /// Gives path to the root of the applicative
    /// In MAC: /path/to/Starlab.app/
    /// In WIN: /path/to/Starlab/
    static QString getBaseDirPath();
    
    /// Assemble string for file open/save filters
    QString getFilterStrings();
    
private:
    /// Searches pluginsDir for plugins
    void loadPlugins();

/// Set of helper functions
private:
    bool load_InputOutputPlugin(QObject* plugin);
    bool load_ProjectInputOutputPlugin(QObject* plugin);
    bool load_FilterPlugin(QObject* plugin);
    bool load_EditPlugin(QObject* plugin);
    bool load_DecoratePlugin(QObject* plugin);    
    bool load_GuiPlugin(QObject* _plugin);
    bool load_DrawAreaPlugin(QObject* _plugin);
    bool load_RenderPlugin(QObject* _plugin);
};

} // namespace
