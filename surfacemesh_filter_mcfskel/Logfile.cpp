#include "Logfile.h"

#include <QString>
#include <QFile>
#include <QElapsedTimer>
#include <QDebug>
#include <QTime>

/// Static variable have local scope!!
/// i.e. only visible by methods in this file
static QFile logfile;
static bool inprogress = false;
static QString agent = "agent";
static QElapsedTimer timer;

void tictocreset(QString filename){
    logfile.setFileName(filename);
    if(logfile.exists()) logfile.remove();
    logfile.open(QFile::WriteOnly);
    logfile.close();
}

void logme(QString message){
    logfile.open(QFile::Append);
    logfile.write(qPrintable(message));
    logfile.write("\n");
    logfile.close();
}

void toc(bool enabled){
    Q_ASSERT(logfile.exists());
    int time = timer.elapsed();
    if(enabled){
        logfile.open(QFile::Append);
        QString message;
        message.sprintf("[%s]\t%dms\n",qPrintable(agent),time);
        logfile.write(qPrintable(message));
        logfile.close();
    }
    inprogress = false;
}

void tic(QString curragent){
    Q_ASSERT(logfile.exists());
    if(inprogress) toc(true);
    agent = curragent;
    timer.start();
}
