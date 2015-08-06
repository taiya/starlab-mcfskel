#pragma once 
#include "StarlabPlugin.h"
#include "StarlabDrawArea.h"

/**
 * @brief The interface of a plugin which gives you total control over the GUI.           
 * @ingroup stariface 
 *
 * @note If you plan to create a widget you should use a ModePluginDockWidget
 */
class STARLIB_EXPORT ModePlugin : public StarlabPlugin {
    Q_OBJECT
    
public: 
/// @brief can the plugin be used?
virtual bool isApplicable() = 0;

/// @{ @name Plugin Creation/Termination
public:
    /// Called when the user starts the edit the FIRST time (i.e. it does not get re-loaded when the plugin is suspended)
    /// Think about it as a constructor, as the plugin constructor would be automatically loaded at boot time.
    virtual void create(){}
    /// Called when the user closes the edit
    /// Automatically closes the widget
    /// @todo is suspend a close?
    /// @internal never use this in the starlab backend!!
    virtual void destroy(){}

    /// Called whenever the document is changed 
    /// If you return false (or if you don't overload this function) we will 
    /// just call ModePlugin::destroy() and ModePlugin::create() in succession.
    virtual bool documentChanged(){ return false; }
protected:
    /// Inside your create initialize any QObject subclass with this parent for auto memory management
    /// @internal I use QWidget instead of QObject so that I can automate Widget memory management as well
    QWidget* parent;
    ModePlugin() :parent(NULL){}
private:
    friend class Starlab::MainWindow;
    friend class gui_mode;
    void __internal_create(){
        Q_ASSERT(parent==NULL);
        parent = new QWidget(mainWindow());
        create();
    }
    void __internal_destroy(){ 
        Q_ASSERT(parent!=NULL);
        delete parent;
        parent = NULL;
        destroy();
    }
/// @} 

/// @{ @name Support for suspension/resume
public:
    /// Called whenever the plugin is suspended (i.e. ESCAPE)
    /// If you created GUIs, hide or disable them here
    virtual void suspend(){}
    /// Called whenever the plugin is resumed (after a suspend)
    /// If you created GUIs, hide or disable them here
    virtual void resume(){}
/// @}

/// @{ @name Rendering events (optional)
public: 
    /// Overload this method to draw *additionally* to the system renderer
    /// @internal never use this in the starlab backend!!
    virtual void decorate(){ emit decorateNeeded(); }
signals:
    /// When gui needs decoration this signal will be called
    void decorateNeeded();
/// @} 

/// @{ @name Selection events (optional)
public:
    /// @see QGLViewer::drawWithNames()
    virtual void drawWithNames(){}
    /// Override to change QGLViewer's default behavior
    /// @return false (i.e. not filtered) causes QGLViewer::endSelection to be called 
    virtual bool endSelection(const QPoint& point){ Q_UNUSED(point); return false; }
    /// Override to change QGLViewer's default behavior
    /// @return false (i.e. not filtered) causes QGLViewer::endSelection to be called 
    virtual bool postSelection(const QPoint& point){ Q_UNUSED(point); return false; }
/// @}

/// @{ @name User input callbacks (optional, return value tells whether you *filtered* the event)
public: 
    virtual bool mousePressEvent        (QMouseEvent* ) { return false; }
    virtual bool mouseMoveEvent         (QMouseEvent* ) { return false; }
    virtual bool mouseReleaseEvent      (QMouseEvent* ) { return false; }
    virtual bool keyReleaseEvent        (QKeyEvent*   ) { return false; }
    virtual bool keyPressEvent          (QKeyEvent*   ) { return false; }
    virtual bool tabletEvent            (QTabletEvent*) { return false; }
    virtual bool wheelEvent             (QWheelEvent* ) { return false; }
    virtual bool mouseDoubleClickEvent  (QMouseEvent* ) { return false; }
/// @}

/// @{ Access to properties
public:
    using StarlabPlugin::drawArea;
    using StarlabPlugin::document;
    using StarlabPlugin::mainWindow;
    using StarlabPlugin::application;
    using StarlabPlugin::pluginManager;
    Starlab::Model* selection(){ return document()->selectedModel(); }    
/// @}    
};

Q_DECLARE_INTERFACE(ModePlugin, "Starlab::ModePlugin/2.0")
