#pragma once
#include "starlib_global.h"
#include "Document.h"
#include "PluginManager.h"
#include "StarlabSettings.h"

namespace Starlab{

/// @note QApplication is for GUI, QCoreApplication is for command line stuff
class STARLIB_EXPORT Application{
    
    /// @{ core resources and accessors
    public:
        PluginManager* pluginManager(){ return _pluginManager; }
        Document* document(){ return _document; }
        Settings* settings(){ return _settings; }
    private:
        PluginManager*   _pluginManager; /// The plugins system
        Document*        _document;      /// The set of loaded models
        Settings* _settings;      /// The application settings
    /// @}
        
public:
    Application();   
    ~Application();
    
public:
    /// @{ I/O: return false if don't know how to load
        void load(QString path);
        bool loadModel(QString path, InputOutputPlugin* plugin=NULL);
        bool saveModel(Model* model, QString path="");
        bool loadProject(QString path, ProjectInputOutputPlugin* plugin=NULL);
    /// @}

    /// @{ filters frontend
        /// Provides the names of all filters
        QList<FilterPlugin*> applicableFilters();
        /// Provides the names of all filters suitable for given model
        QList<FilterPlugin*> applicableFilters(Model* model);
        /// Executes a filter by name
        void executeFilter(Model *model, QString filterName);
    /// @}
    
    /// Where is Starlab located?
    /// Mac: /Applications/Starlab.app
    /// Win: C:/Program Files/starlab
    /// Lin: ????
    QDir starlabDirectory();
    
    /// Where was starlab executed from?
    QDir executionDirectory();
    
    static QString version(){ return "1.0.4 alpha"; }
};

} // Namespace
