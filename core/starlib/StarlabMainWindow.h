#pragma once
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QStatusBar>
#include <QProgressBar>
#include <QDragEnterEvent>
#include <QToolBar>

#include "Starlab.h"
#include "Document.h"
#include "StarlabException.h"
#include "StarlabSettings.h"
#include "StarlabApplication.h"
#include "starlib_global.h"

namespace Starlab{


/** 
 * @brief The definition of the main starlab GUI
 * @ingroup starcore
 */
/// The basic structure of the stalab window to which plugins (i.e. GUI plugins) interface
class STARLIB_EXPORT MainWindow : public QMainWindow{
    Q_OBJECT
    

    
/// @{ Core
public:
    MainWindow(Application *_application);
    ~MainWindow();
public: 
    Application* application(){ return _application; }
    /// The global settings of the application
    Settings* settings(){ return application()->settings(); }
    /// Contains the document (extracted from application)
    Document* document(){ return application()->document(); }
    /// Contains the loaded plugins (extracted from application)
    PluginManager* pluginManager(){ return application()->pluginManager(); }   
private:
    /// Contains common (non-gui) functionalities
    Application* const _application;
/// @}

    
    
    
/// @{ Basic GUI behavior
    /// Quit whole application whenever the main window is closed
    void closeEvent(QCloseEvent *);        
    
private slots:
    /// @brief Update the whole Starlab Window (reloads toolbar & menus)
    /// @note See drawArea if that's what you want to update
    void update();
    
/// @}
    
    
    
    
/// @{ DrawArea Management (Center of Starlab window) 
private:
    /// Manager of main drawing area (MeshLab: GLArea)
    DrawArea* _drawArea; /// NULL
public:
    /// Returns pointer to the active draw area
    DrawArea* drawArea(){ return _drawArea; }
/// @}
           
    
    
    
/// @{ ModePlugin Management
private:
    ModePlugin* _modePlugin;
    bool _isModePluginSuspended;
signals:
    void modePluginSet(ModePlugin*);
public:
    bool isModePluginSuspended(){ Q_ASSERT(_modePlugin); return _isModePluginSuspended; }
    void resumeModePlugin(){ Q_ASSERT(_modePlugin); _isModePluginSuspended=false; }
    void suspendModePlugin(){ Q_ASSERT(_modePlugin); _isModePluginSuspended=true; }
    bool hasModePlugin(){ return (_modePlugin!=NULL); }
    /// Sets the given mode plugin as the active 
    /// @pre mode!=NULL
    /// @post the given plugin becomes active within the system
    /// @emit modePluginSet(mode)
    void setModePlugin(ModePlugin* mode);
    void removeModePlugin(){ Q_ASSERT(_modePlugin); _modePlugin=NULL; }
    ModePlugin* getModePlugin(){ Q_ASSERT(_modePlugin); return _modePlugin; }
/// @}
    
    
    
    
/// @{ @name Window's menus (in order)
public:
    QMenu *preferencesMenu; ///< @todo settings menu, refer to MeshLab
    QMenu *fileMenu;        ///< @todo
    QMenu *modeMenu;        ///< EditPlugin entries are stored here
    QMenu *filterMenu;      ///< FilterPlugin entries are stored here
    QMenu *renderMenu;      ///< RenderPlugin entries are stored here
#ifdef ENABLE_DECORATION
    QMenu *decorateMenu;    ///< DecoratePlugin entries are stored here
#endif
    QMenu *viewMenu;        ///< StarlabDrawArea component responsible for this menu
    QMenu *windowsMenu;
    QMenu *helpMenu;
private:
    /// @todo is there a better way to retrieve all sub-menus? I attempted "foreach(QAction* menu_a, menuBar()->actions()){...}"
    QList<QMenu*> menus;    ///< List of pointers to all the above
/// @}

    
    
    
/// @{ @name Window's toolbars (in order)
public:
    QToolBar *mainToolbar;      ///< Core icons, always visible regardless
    QToolBar *modeToolbar;      ///< Edit-specific icons, only added by edits which....QMenu *fileMenu;           
    QToolBar *filterToolbar;    ///< Filter-specific icons, only added by filters which....
    QToolBar *renderToolbar;    ///< Toolbar icons for rendering
#ifdef ENABLE_DECORATION
    QToolBar *decorateToolbar;  ///< Toolbar icons for decorations
#endif
private:
    /// @todo is there a better way to retrieve all sub-toolbars?    
    QList<QToolBar*> toolbars; ///< List of all available toolbars
/// @}
    
    
    
    
/// @{ @name automatic action trigger
/// @todo is there a way of making these global variables?
public slots:
    /// @brief Executes a filter by name, achieving the same effect as clicking on an action. 
    /// @todo what if two filters have the same name?
    void triggerFilterByName(QString name);

    /// @brief Executes a menu command by name, achieving the same effect as clicking on an action. 
    void triggerMenuActionByName(QString name);
/// @}
    
    
    
    
/// @{ @name status bar management
public slots:
    /// Show message on statusbar, default timeout is 2 seconds
    /// Message stays permanent with timeout_seconds set to 0
    void setStatusBarMessage(QString message, double timeout_seconds=2.0);
    /// Sets statusbar progress to a certain value
    /// @par Who is performing an action?
    /// @par What's the completion? [0..1]
    void setProgressBarValue(double completion);
    void closeProgressBar();        
private slots: 
    void hideToolbarOnEmptyMessage(QString message);
    /// @brief notifies changes in selection with a message on the statusbar
    /// @todo  also change base color of model for an instant!
    void selectionChanged(Model* selected);
private:
    QStatusBar*      _statusBar; ///< This should not be touched, use this->setStatusBarMessage
    QProgressBar*  _progressBar; ///< This should not be touched, use this->setProgressBarPercentage
    QList<QString> _oldMessages; ///< Saves all messages sent to statusbar
/// @}

    
    
    
/// @{ @name Core Events Management
public slots:
    /// Manages Drag & Drop events
    void dropEvent(QDropEvent* event);
    /// @todo precisely why?
    void dragEnterEvent(QDragEnterEvent* event){ event->accept(); }
    /// This slot receives an action and displays a tooltip for it at the current mouse position
/// @}

      
    
    
/// @{ @name GUI elements
private:
    /// i.e. typically connected to the hovered(QAction*) signal to show tooltips
    // void showActionTooltip(QAction*action);
public:
    /// Determines default window size
    QSize sizeHint() const;
/// @} 
    
};
 
} // namespace

typedef Starlab::MainWindow StarlabMainWindow;
typedef Starlab::MainWindow SMainWindow;

Q_DECLARE_INTERFACE(Starlab::MainWindow, "Starlab::MainWindow/2.0")
