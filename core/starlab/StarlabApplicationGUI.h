#pragma once
#include <QApplication>
#include <QKeyEvent>
#include <QDebug>
#include "StarlabException.h"


class StarlabApplicationGUI : public QApplication{

public:
    StarlabApplicationGUI(int& argc, char* argv[]) : QApplication(argc,argv){
        QLocale::setDefault(QLocale::C); // Use "C"urrent locale
        setOrganizationName("Free Software Foundation");
        setApplicationName("Starlab");
        setApplicationVersion("1.0.1");
        setQuitOnLastWindowClosed(true);
    }

    /// @todo why is this necessary?
    bool notify( QObject * rec, QEvent * ev ){
        try{
            return QApplication::notify(rec,ev);
        } catch (StarlabException& e){        
            QString title = "Unmanaged StarLab Exception";
            QString message = e.message();
            int retval = QMessageBox::critical(NULL, title, message, QMessageBox::Ignore|QMessageBox::Abort);
            if( retval == QMessageBox::Abort ){
                qCritical() << "Terminated because user aborted on raised exception";
                // this->closeAllWindows();
                this->exit(0);
            }
            /// Restore pointer if it was changed
            qApp->restoreOverrideCursor();
        } catch(std::exception& e){ \
            qWarning() << "[std::exception]" << e.what();
            qWarning() << "[TERMINATING]";
            exit(-1);
        } catch (...) {
            qDebug() << "BAD EXCEPTION!!!";
            qWarning() << "[TERMINATING]";
            exit(-1);
        }
        // the event will be propagated to the receiver's parent and 
        // so on up to the top-level object if the receiver is not 
        // interested in the event (i.e., it returns false).
        return false;
    }    
};
