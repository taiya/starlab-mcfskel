#include "plugin_gui_helpmenu.h"
Q_EXPORT_PLUGIN(plugin_gui_helpmenu)

#if false
void during_main_init(){
    {
        httpReq=new QHttp(this);
        //connect(httpReq, SIGNAL(requestFinished(int,bool)), this, SLOT(connectionFinished(int,bool)));
        connect(httpReq, SIGNAL(done(bool)), this, SLOT(connectionDone(bool)));
    }
}

void MainWindow::connectionDone(bool /* status */) {
    QString answer=myLocalBuf.data();
    if(answer.left(3)==QString("NEW"))
        QMessageBox::information(this,"MeshLab Version Checking",answer.remove(0,3));
    else if (VerboseCheckingFlag) QMessageBox::information(this,"MeshLab Version Checking","Your MeshLab version is the most recent one.");

    myLocalBuf.close();
    QSettings settings;
    int loadedMeshCounter=settings.value("loadedMeshCounter",0).toInt();
    settings.setValue("lastComunicatedValue",loadedMeshCounter);
}

void MainWindow::checkForUpdates(bool verboseFlag) {
    VerboseCheckingFlag=verboseFlag;
    QSettings settings;
    int totalKV=settings.value("totalKV",0).toInt();
    int connectionInterval=settings.value("connectionInterval",20).toInt();
    settings.setValue("connectionInterval",connectionInterval);
    int loadedMeshCounter=settings.value("loadedMeshCounter",0).toInt();
    int savedMeshCounter=settings.value("savedMeshCounter",0).toInt();
    QString UID=settings.value("UID",QString("")).toString();
    if(UID.isEmpty()) {
        UID=QUuid::createUuid ().toString();
        settings.setValue("UID",UID);
    }

#ifdef _DEBUG_PHP
    QString BaseCommand("/~cignoni/meshlab_d.php");
#else
    QString BaseCommand("/~cignoni/meshlab.php");
#endif

#ifdef Q_WS_WIN
    QString OS="Win";
#elif defined( Q_WS_MAC)
    QString OS="Mac";
#else
    QString OS="Lin";
#endif
    QString message=BaseCommand+QString("?code=%1&count=%2&scount=%3&totkv=%4&ver=%5&os=%6").arg(UID).arg(loadedMeshCounter).arg(savedMeshCounter).arg(totalKV).arg(appVer()).arg(OS);
    idHost=httpReq->setHost("vcg.isti.cnr.it"); // id == 1
    bool ret=myLocalBuf.open(QBuffer::WriteOnly);
    if(!ret) QMessageBox::information(this,"Meshlab",QString("Failed opening of internal buffer"));
    idGet=httpReq->get(message,&myLocalBuf);     // id == 2

}
#endif

#if 0
void MainWindow::submitBug() {
    QMessageBox mb(QMessageBox::NoIcon,tr("MeshLab"),tr("MeshLab"),QMessageBox::NoButton, this);
    //mb.setWindowTitle(tr("MeshLab"));
    QPushButton *submitBug = mb.addButton("Submit Bug",QMessageBox::AcceptRole);
    mb.addButton(QMessageBox::Cancel);
    mb.setText(tr("If Meshlab closed in unexpected way (e.g. it crashed badly) and"
                  "if you are able to repeat the bug, please consider to submit a report using the SourceForge tracking system.\n"
                 ) );
    mb.setInformativeText(	tr(
                                "Hints for a good, useful bug report:\n"
                                "- Be verbose and descriptive\n"
                                "- Report meshlab version and OS\n"
                                "- Describe the sequence of actions that bring you to the crash.\n"
                                "- Consider submitting the mesh file causing a particular crash.\n"
                            ) );

    mb.exec();

    if (mb.clickedButton() == submitBug)
        QDesktopServices::openUrl(QUrl("http://sourceforge.net/tracker/?func=add&group_id=149444&atid=774731"));

}
#endif

#if false
void MainWindow::about() {
    QDialog *about_dialog = new QDialog();
    Ui::aboutDialog temp;
    temp.setupUi(about_dialog);
    temp.labelMLName->setText(appName()+"   ("+__DATE__+")");
    //about_dialog->setFixedSize(566,580);
    about_dialog->show();
}

void MainWindow::aboutPlugins() {
    qDebug( "aboutPlugins(): Current Plugins Dir: %s ",qPrintable(pluginManager.getPluginDirPath()));
    PluginDialog dialog(pluginManager.getPluginDirPath(), pluginManager.loadedPlugins.keys(), this);
    dialog.exec();
}

void MainWindow::helpOnscreen() {
    if(GLA()) GLA()->toggleHelpVisible();
}

void MainWindow::helpOnline() {
    checkForUpdates(false);
    QDesktopServices::openUrl(QUrl("http://meshlab.sourceforge.net/wiki"));
}
#endif
