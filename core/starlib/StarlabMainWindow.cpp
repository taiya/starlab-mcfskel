#include "StarlabMainWindow.h"

#include <QGLWidget>
#include <QFileOpenEvent>
#include <QToolBar>
#include <QDesktopWidget>
#include <QUrl> /// Drag&Drop
#include <QApplication>
#include <QToolTip>
#include <QMimeData>

#include "GuiPlugin.h"
#include "StarlabDrawArea.h"
#include "ModePlugin.h"

using namespace Starlab;

MainWindow::MainWindow(Application* _application) :
    _application(_application)
{   
    /// Init
    _modePlugin = NULL;
    _isModePluginSuspended = false;
    
    /// Setup central widget
    {
        // Anti-aliasing when using QGLWidget or subclasses
        QGLFormat glf = QGLFormat::defaultFormat();
        glf.setSamples(8);
#if 0
        /// Set a 32 bits depth buffer
        glf.setDepth(true);
        glf.setDepthBufferSize(32);
#endif
        QGLFormat::setDefaultFormat(glf);

        _drawArea = new DrawArea(this);
        this->setCentralWidget(_drawArea);
        _drawArea->setAcceptDrops(true);
    }
    
    /// Register all plugins with the main window
    foreach(StarlabPlugin* plugin, pluginManager()->plugins()){
        plugin->_mainWindow = this;
    }
    
    
    /// Sets window icon/name
    {
        /// Put a little icon in the window title
        // QIcon icon;
        // icon.addPixmap(QPixmap(":images/___.png"));
        // setWindowIcon(icon);
        setWindowTitle("Starlab v1.0 (alpha)");        
    }
       
    /// Instantiate Menus (plugins will fill them in)
    /// Do not use the silly "&" symbols for windows notation for alt navigation
    {
        menus << (fileMenu      = menuBar()->addMenu("File"));
        menus << (modeMenu      = menuBar()->addMenu("Mode"));
        menus << (filterMenu    = menuBar()->addMenu("Filters"));
        menus << (renderMenu    = menuBar()->addMenu("Render"));
#ifdef ENABLE_DECORATION
        menus << (decorateMenu  = menuBar()->addMenu("Decorate"));
#endif
        menus << (viewMenu      = menuBar()->addMenu("View"));
        menus << (windowsMenu   = menuBar()->addMenu("Windows"));
        menus << (helpMenu      = menuBar()->addMenu("Help"));
       
/// @todo this was annoying and caused bugs, temporarily disabled
#ifdef TODO_SHOW_ACTION_TOOLTIP
        /// Setup tooltips (mouse hover) for menu entries
        foreach(QMenu* menu, menus)
            connect(menu,SIGNAL(hovered(QAction*)),this,SLOT(showActionTooltip(QAction*)));
#endif
    }
    
    /// Instantiate Toolbars
    {
        toolbars << ( mainToolbar = addToolBar(tr("Main Toolbar")) );
        toolbars << ( modeToolbar = addToolBar(tr("Mode Toolbar")) );
#ifdef ENABLE_DECORATION
        toolbars << (decorateToolbar = addToolBar(tr("Decorate Toolbar")) );
#endif
        toolbars << (filterToolbar = addToolBar(tr("Filter Toolbar")) );
        toolbars << (renderToolbar = addToolBar(tr("Render Toolbar")) );
                        
        /// Initially hide toolbars
        mainToolbar->setVisible(true);
        modeToolbar->setVisible(false);
        renderToolbar->setVisible(false);
#ifdef ENABLE_DECORATION       
        decorateToolbar->setVisible(false);
#endif
        filterToolbar->setVisible(false);
    }
    
    /// Sets up progress/status bar    
    {
        _progressBar = new QProgressBar(this);
        _statusBar   = new QStatusBar(this);
        this->setStatusBar(_statusBar);
        _progressBar->setMaximum(100);
        _progressBar->setMinimum(0);
        _progressBar->setVisible(false);
        _statusBar->addPermanentWidget(_progressBar,0);
        /// By default bottom statusbar is invisible
        _statusBar->setVisible(false);       
        
        /// @brief Whenever there is nothing to show, hide the toolbar.
        /// @todo add a timer that disables it after a while otherwise it's kind of annoying
        connect(_statusBar, SIGNAL(messageChanged(QString)), this, SLOT(hideToolbarOnEmptyMessage(QString)) );
        
        /// Show visual notification for changes in selection
        connect(document(), SIGNAL(selectionChanged(Model*)), this, SLOT(selectionChanged(Model*)));
    }
    
    /// Intercepts the "open" events sent directly by the Operative System in this->eventFilter
    {
        setAcceptDrops(true);
    }
       
    /// Connect document changes to view changes
    {
        connect(document(),SIGNAL(resetViewport()), drawArea(),SLOT(resetViewport()));
        connect(document(),SIGNAL(resetViewport()), drawArea(),SLOT(updateGL()));
    }
    
    /// Installs all the GUI plugins
    {
        foreach(GuiPlugin* plugin, pluginManager()->guiPlugins()){
            // qDebug() << "GuiPlugin::load " << plugin->name();
            plugin->_mainWindow=this;
            plugin->load();
        }
    }
        
    /// Make sure settings are fresh & inform user on where settings are loaded from
    {
        settings()->sync();
        setStatusBarMessage("Settings loaded from: "+settings()->settingsFilePath(),2);
    }

    /// Update the UI for the first time (e.g. load mode plugins into UI)
    update();

    /// Delete MainWindow when it's closed    
    this->setAttribute(Qt::WA_DeleteOnClose, true);
}

MainWindow::~MainWindow(){
    // qDebug() << "~MainWindow()";
    if(hasModePlugin())
        getModePlugin()->__internal_destroy();
}

void MainWindow::closeEvent(QCloseEvent*){
    QApplication::exit(0);
}

void MainWindow::dropEvent(QDropEvent* event) {
    // qDebug() << "StarlabMainWindow::dropEvent()";
    const QMimeData* data = event->mimeData();
    if(data->hasUrls())
        foreach(QUrl url, data->urls())
            QApplication::sendEvent(this, new QFileOpenEvent(url));    
}


#ifdef TODO_SHOW_ACTION_TOOLTIP
void StarlabMainWindow::showActionTooltip(QAction* action){
    /// @todo can we have the tooltip appear with a delay?
    QString tip = action->toolTip();
    if(!tip.isNull())
        QToolTip::showText(QCursor::pos(), tip);
}
#endif

QSize MainWindow::sizeHint() const{
    QRect geom = QApplication::desktop()->screenGeometry();
    int scrw = geom.width();
    int scrh = geom.height();   
    return QSize(scrw/2,scrh/2);
}

void MainWindow::triggerFilterByName(QString name){
    name = name.toLower();
    setStatusBarMessage("Auto-triggering filter: '" + name + "'.");
    bool isFound=false;   
    
    /// Search the "filter" menu for the proper plugin
    foreach(QAction* action, filterMenu->actions())
        if(action->text()==name){
            isFound=true;
            action->trigger();
        }
    
    if(!isFound){
        setStatusBarMessage("Filter '"+name+"' was requested but could not be found.");
    }
}

void MainWindow::setModePlugin(ModePlugin* mode){ 
    Q_ASSERT(_modePlugin==NULL); 
    _modePlugin=mode;
    emit modePluginSet(mode);
}

/// @todo Implement this function...
void MainWindow::triggerMenuActionByName(QString name){
    QAction* trigger_me = NULL;
    foreach(QMenu* menu, menus)
        foreach(QAction* action, menu->actions())
            if(action->text()==name)
                trigger_me = action;
    
    if(trigger_me){
        setStatusBarMessage("Auto-Started: '"+ name + "'",10);
        trigger_me->trigger();
    }
    else
        setStatusBarMessage("[WARNING] Auto-Started failed: '"+ name + "'",10);
}

void MainWindow::update(){
    // qDebug() << "StarlabMainWindow::update()";
    
    /// Update all the menus/toolbars
    foreach(GuiPlugin* plugin, pluginManager()->guiPlugins()){
        // qDebug() << "updating: " << plugin->name();
        plugin->update();
    }
      
    /// Only display menus and toolbars if they contain something
    {        
        /// Disable empty menus...
#if 0
        foreach(QMenu* menu, menus){
            // qDebug() << menu->title() << hasactions;
            bool hasactions = menu->actions().size()>0;
            menu->menuAction()->setVisible(hasactions);
            qDebug() << "[MENU]" << menu->title() << hasactions;
        }
#endif
        
        /// Disable empty toolbars...
        foreach(QToolBar* tb, toolbars){
            bool hasactions = tb->actions().size()>0;
            tb->setVisible(hasactions);
        }
    }
}

void MainWindow::setStatusBarMessage(QString message, double timeout_seconds){
    /// Setup tooltip of old messages
    _oldMessages.prepend(message);
    QString tooltipMessage = "Message Log (first is recent):";
    foreach(QString old, _oldMessages) tooltipMessage.append("\n-> "+old);
    _statusBar->setToolTip(tooltipMessage);
    /// Show the current message
    _statusBar->showMessage(message, (int)timeout_seconds*1000);
    _statusBar->show();
    _statusBar->update();
    /// Make sure we can see something, redraw event processed
    /// http://lists.trolltech.com/qt-interest/2008-08/msg00325.html
    QApplication::processEvents();
}

void MainWindow::setProgressBarValue(double completion){
    completion = qBound(completion, 0.0, 1.0);
    // _statusBar->showMessage(actor); ///< @todo is 0 timeout appropriate?    
    _progressBar->setEnabled(true);
    _progressBar->setValue(completion*100);
    _progressBar->show();
    /// Make sure we can see something, redraw event processed
    /// http://lists.trolltech.com/qt-interest/2008-08/msg00325.html
    QApplication::processEvents();
}

void MainWindow::closeProgressBar(){
    _progressBar->hide();
    _progressBar->reset();
}

void MainWindow::hideToolbarOnEmptyMessage(QString /*message*/){
    // qDebug() << "NEW MESSAGE: " << message;
    /// @todo disabled as it's annoying to see it popping in/out
    // if(message.isEmpty())  _statusBar->hide();
}

void MainWindow::selectionChanged(Model *selected){
    QString name = (selected==NULL) ? "No Selection" : selected->name;
    this->setStatusBarMessage("Selected model has been changed to '" + name + "'", 3);
}
