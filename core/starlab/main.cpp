#include <QDebug>
#include <QColor>
#include <QGLFormat> /// @todo why in main.cpp???
#include "StarlabMainWindow.h"
#include "StarlabApplicationGUI.h"
#include "StarlabApplication.h"
#include "StarlabSettings.h"
#include "FileOpenEater.h"
#include "interfaces/FilterPlugin.h"

using namespace Starlab;

int main(int argc, char *argv[]) { 
	/// Create QT GUI app
	StarlabApplicationGUI app(argc,argv);

	/// Create a starlab application
	Application application;    

	/// Create a new window (@internal *new* is important)
	MainWindow* mainWindow = new MainWindow(&application);

	/// Manages I/O requested by Operating system
	FileOpenEater* eater = new FileOpenEater(mainWindow);
	mainWindow->installEventFilter(eater);

	/// Open command line input
	for(int i=1; i<argc; i++)
	    QApplication::sendEvent(mainWindow, new QFileOpenEvent(argv[i]));

	/// Automatically load layer menu if I opened more than one model    
	if(mainWindow->document()->models().size()>=2)
	    mainWindow->triggerMenuActionByName("Show Layers Dialog");    

	/// Show the window 
	/// (@internal see QWidget::activateWindow)
	mainWindow->showNormal();
	mainWindow->activateWindow();
	mainWindow->raise();

	/// Auto-start a user-specified action
	QString actionName = application.settings()->getString("autostartWithAction");
	if(!actionName.isEmpty()) mainWindow->triggerMenuActionByName(actionName);

	/// Starts the event loop
	return app.exec();    
}
