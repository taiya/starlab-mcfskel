#include "gui_python.h"
#include <QPalette>

#include "PythonQt.h"
#include "gui/PythonQtScriptingConsole.h"

void gui_python::load(){
    
    /// Create console gui
    {        
        /// Create widget
        widget = new QDockWidget("Python Console", mainWindow());
        widget->setVisible(false);
        widget->setAllowedAreas(Qt::BottomDockWidgetArea);
        
        /// Create console
        PythonQt::init(PythonQt::IgnoreSiteModule | PythonQt::RedirectStdOut);
        PythonQtObjectPtr mainContext = PythonQt::self()->getMainModule();
        PythonQtScriptingConsole* console = new PythonQtScriptingConsole(widget, mainContext);
        widget->setWidget(console);
        
        /// Set the console background color
        QPalette palette =  console->palette();
        palette.setColor(QPalette::Base,       QColor( 75,  75,  75));
        palette.setColor(QPalette::Text,       QColor(  0, 255,   0)); 
        console->setPalette(palette);
        console->show();

        /// Add basic components to console
        mainContext.addObject("drawArea", (QObject*) drawArea());
        
        /// Add it to the main window (does not show)
        mainWindow()->addDockWidget(Qt::BottomDockWidgetArea, widget);
    }
    
    /// Action to pop up the terminal
    {
        QAction* action = new QAction(QIcon(":/icons/python.png"),tr("Show Python Console"), this);
        action->setCheckable(true);
        action->setChecked(false);
        toolbar()->addAction(action);
        menu()->addAction(action);
        connect(action, SIGNAL(triggered(bool)), widget, SLOT(setVisible(bool)));
    }
    
    /// Connect edit plugins to console
    {
        /// If a plugin was already loaded, hook it up
        if( mainWindow()->hasModePlugin() )
            modePluginSetSlot( mainWindow()->getModePlugin() );

        /// In any cause, when stuff changes, update the variable
        connect( mainWindow(), SIGNAL(modePluginSet(ModePlugin*)), this, SLOT(modePluginSetSlot(ModePlugin*)) );
    }
}

void gui_python::modePluginSetSlot(ModePlugin* mode){
    PythonQtObjectPtr mainContext = PythonQt::self()->getMainModule();
    mainContext.removeVariable("mode");
    mainContext.addObject("mode", (QObject*) mode);
}

Q_EXPORT_PLUGIN(gui_python)

//int main( int argc, char **argv ){
//  QApplication qapp(argc, argv);
//  PythonQt::init(PythonQt::IgnoreSiteModule | PythonQt::RedirectStdOut);
//  PythonQtObjectPtr  mainContext = PythonQt::self()->getMainModule();
//  PythonQtScriptingConsole console(NULL, mainContext);
//  // add a QObject to the namespace of the main python context
//  PyExampleObject example;
//  mainContext.addObject("example", &example);
//  // mainContext.evalFile(":example.py");
//  console.show();
//  return qapp.exec();
//}
