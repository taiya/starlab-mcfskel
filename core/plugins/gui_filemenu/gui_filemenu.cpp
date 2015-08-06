#include "gui_filemenu.h"
Q_EXPORT_PLUGIN(gui_filemenu)

#include <QFileInfo>
#include "StarlabDrawArea.h"
#include "OSQuery.h"

const static QString all_files = "All Files (*.*)";

using namespace Starlab;

void gui_filemenu::delete_selected_model(){
    if(selectedModel()==NULL)
        return;
    document()->deleteModel( selectedModel() );
}

void gui_filemenu::open(){
    /// Restore browsing directory from the cache
    QDir lastUsedDirectory( settings()->getString("lastUsedDirectory") );
    
    /// Builds allowed extensions / filter list
    QString filters = all_files;
    {
        QTextStream sout(&filters);
        foreach(InputOutputPlugin* plugin, pluginManager()->modelIOPlugins)
            sout << ";;" << plugin->name();
        foreach(ProjectInputOutputPlugin* plugin, pluginManager()->projectIOPlugins)
            sout << ";;" << plugin->name();
    }
    
    /// Prompt user for file to open
    QString selectedFilter;
    QString fileName = QFileDialog::getOpenFileName(mainWindow(),tr("Open Project File"), lastUsedDirectory.path(),filters,&selectedFilter);
    // QString extension = QFileInfo(fileName).suffix().toLower();
    
    /// "Cancel" button was pressed
    if(fileName.isNull()) return;
    
    /// Cache the opened directory for future use
    /// do it early so if operation fail it's still cached
    QFileInfo fileInfo(fileName);
    settings()->set( "lastUsedDirectory", fileInfo.absolutePath() );
    
    /// If user is trustring the "auto-loader"
    if(selectedFilter == all_files)
        application()->load(fileName);
    
    /// But if he preferred something else
    else{
        InputOutputPlugin* model_plugin = pluginManager()->modelIOPlugins.value(selectedFilter,NULL);
        ProjectInputOutputPlugin* project_plugin = pluginManager()->projectExtensionToPlugin.value(selectedFilter,NULL);
        Q_ASSERT(model_plugin==NULL || project_plugin==NULL);
    
        if(project_plugin != NULL) application()->loadProject(fileName,project_plugin);
        if(model_plugin != NULL) application()->loadModel(fileName,model_plugin);
    }
}

/// Case 1: only one model (didn't exist) and doc never existed
/// Case 2: only one model (did exist) and doc never existed
/// Case 3: multiple models but doc never existed
/// Case 4: multiple models and doc already existed
void gui_filemenu::save(){
    try 
    {
        // qDebug() << "[[ENTERING]] gui_filemenu::save()";
        if(document()->models().size()==0) return;        
        
        if(true){
            Starlab::Model* model = document()->selectedModel();
            bool pathAlreadySpecified = (model->path != "");
            bool success = false;
            
            /// Already know where to save
            if( pathAlreadySpecified )
                success = application()->saveModel(model, model->path);
            
            /// Query user where to save & save in model path
            else{
                QString lastDir = settings()->getString("lastUsedDirectory");
                QString fileName = QFileDialog::getSaveFileName(mainWindow(),"Save Selected Model",lastDir);
                if(fileName.isEmpty()) return;
                model->path = fileName;
                model->name = QFileInfo(fileName).baseName();
                success = application()->saveModel(model);
            }
            
            if(success){
                QFileInfo finfo(model->path);
                mainWindow()->setStatusBarMessage("Saved model at path: " + finfo.absoluteFilePath(),2.0f);
            } else {            
                mainWindow()->setStatusBarMessage("Save operation failed...",2.0f);
            }
        } else {
            throw StarlabException("gui_file::save() modes 2...4 not implemented");
        }
    }
    STARLAB_CATCH_BLOCK
}


void gui_filemenu::reload_selection(){
    // qDebug() << "gui_filemenu::reload_selection()";
    mainWindow()->document()->pushBusy();
        Model* selection = mainWindow()->document()->selectedModel();
        if(selection==NULL) return;
        QFileInfo fi(selection->path);
        if(!fi.exists())
            throw StarlabException("Cannot reload mode, file %s cannot be found.",selection->path.toStdString().c_str());
    
        /// Guess open plugin by extension    
        QString extension = QFileInfo(selection->path).suffix().toLower();    
        QList<InputOutputPlugin*> plugins = pluginManager()->modelExtensionToPlugin.values(extension);
        
        /// Check which of these have generated the model, then use it to re-open
        Model* newmodel = NULL;
        foreach(InputOutputPlugin* plugin, plugins)
            if(plugin->isApplicable(selection))
                newmodel = plugin->open(selection->path);       
    
        if(newmodel==NULL)
            throw StarlabException("Impossible ot reload model");
        
        /// Compute its BBOX, otherwise rendering will not work
        newmodel->updateBoundingBox();
        
        /// Replace and set as selected model
        mainWindow()->document()->addModel(newmodel);
        mainWindow()->document()->deleteModel(selection);
        mainWindow()->document()->setSelectedModel(newmodel);
    mainWindow()->document()->popBusy();

    /// Inform the user
    mainWindow()->setStatusBarMessage("Model '"+ newmodel->name +"' reloaded from path: " + newmodel->path,5000);
}

void gui_filemenu::save_selection_as(){
    document()->selectedModel()->path = "";
    this->save();
}

void gui_filemenu::take_screenshot(){
    /// Screen Shot 2013-04-03 at 6.59.51 PM
    QString date = QDateTime::currentDateTime().toString("yyyy-MM-dd h.mm.ss AP");
    QString filename = QDir::homePath() + "/Desktop/" + QString("Screen Shot ") + date + ".png";
//    QDir::setCurrent(QDir::homePath());
    
#if 0
    // this uses qglviewer stuff (but no alpha!!)
    // drawArea()->saveSnapshot(filename, true);
#else
    // this uses qt stuff
    // drawArea()->setAttribute(Qt::WA_TranslucentBackground,true);    
    QImage image = drawArea()->grabFrameBuffer(true);
    image.save(filename, "png",100);
    if(drawArea()->backgroundColor().alpha()<255 && drawArea()->format().samples()>1)
        qWarning() << "Antialiasing and transparent backgrounds do work well in snapshots" 
                   << "Change background opacity to 100% in the render menu";
#endif
        
    showMessage("Screenshot saved at: %s",qPrintable(filename));
}

#if 0
QAction *recentFileActs[MAXRECENTFILES];
QAction *recentProjActs[MAXRECENTFILES];
#endif

/// In the constructor
#if 0
for (int i = 0; i < MAXRECENTFILES; ++i) {
    recentProjActs[i] = new QAction(this);
    recentProjActs[i]->setVisible(true);
    recentProjActs[i]->setEnabled(true);

    recentFileActs[i] = new QAction(this);
    recentFileActs[i]->setVisible(true);
    recentFileActs[i]->setEnabled(false);
    recentFileActs[i]->setShortcut(QKeySequence(Qt::ALT + Qt::Key_1+i));
    connect(recentProjActs[i],SIGNAL(triggered()),this,SLOT(openRecentProj()));
    connect(recentFileActs[i], SIGNAL(triggered()),this, SLOT(openRecentMesh()));

}
#endif

/// Allocation
#if 0
    QMenu* recentProjMenu;
    QMenu* recentFileMenu;

    recentProjMenu = fileMenu->addMenu(tr("Recent Projects"));
    recentFileMenu = fileMenu->addMenu(tr("Recent Files"));
    
    for (int i = 0; i < MAXRECENTFILES; ++i) {
        recentProjMenu->addAction(recentProjActs[i]);
        recentFileMenu->addAction(recentFileActs[i]);
    }
#endif

#if 0
    void MainWindow::updateRecentFileActions() {
        bool activeDoc = true; /// Now it's always true...
    
        QSettings settings;
        QStringList files = settings()->value("recentFileList").toStringList();
    
        int numRecentFiles = qMin(files.size(), (int)MAXRECENTFILES);
    
        for (int i = 0; i < numRecentFiles; ++i) {
            QString text = tr("&%1 %2").arg(i + 1).arg(QFileInfo(files[i]).fileName());
            recentFileActs[i]->setText(text);
            recentFileActs[i]->setData(files[i]);
            recentFileActs[i]->setEnabled(activeDoc);
        }
        for (int j = numRecentFiles; j < MAXRECENTFILES; ++j)
            recentFileActs[j]->setVisible(false);
    }
#endif
    
#if 0
void MainWindow::saveRecentProjectList(const QString &projName) {
    QSettings settings;
    QStringList files = settings()->value("recentProjList").toStringList();
    files.removeAll(projName);
    files.prepend(projName);
    while (files.size() > MAXRECENTFILES)
        files.removeLast();

    for(int ii = 0; ii < files.size(); ++ii)
        files[ii] = QDir::fromNativeSeparators(files[ii]);

    settings()->setValue("recentProjList", files);

    foreach (QWidget *widget, QApplication::topLevelWidgets()) {
        MainWindow *mainWin = qobject_cast<MainWindow *>(widget);
        if (mainWin)
            mainWin->updateRecentProjActions();
    }
}
#endif

#if 0
void MainWindow::updateRecentProjActions() {
    //bool activeDoc = (bool) !mdiarea->subWindowList().empty() && mdiarea->currentSubWindow();

    QSettings settings;
    QStringList projs = settings()->value("recentProjList").toStringList();

    int numRecentProjs = qMin(projs.size(), (int)MAXRECENTFILES);
    for (int i = 0; i < numRecentProjs; ++i) {
        QString text = tr("&%1 %2").arg(i + 1).arg(QFileInfo(projs[i]).fileName());
        recentProjActs[i]->setText(text);
        recentProjActs[i]->setData(projs[i]);
        recentProjActs[i]->setEnabled(true);
    }
    for (int j = numRecentProjs; j < MAXRECENTFILES; ++j)
        recentProjActs[j]->setVisible(false);
}
#endif

#if 0
// this function update the app settings with the current recent file list
// and update the loaded mesh counter
void MainWindow::saveRecentFileList(const QString &fileName) {
    QSettings settings;
    QStringList files = settings()->value("recentFileList").toStringList();
    files.removeAll(fileName);
    files.prepend(fileName);
    while (files.size() > MAXRECENTFILES)
        files.removeLast();

    //avoid the slash/back-slash path ambiguity
    for(int ii = 0; ii < files.size(); ++ii)
        files[ii] = QDir::fromNativeSeparators(files[ii]);
    settings()->setValue("recentFileList", files);

    /// @todo
    settings()->setValue("totalKV",          settings()->value("totalKV",0).toInt() /*+(GLA()->mm()->n_vertices())/1000 */);
    settings()->setValue("loadedMeshCounter",settings()->value("loadedMeshCounter",0).toInt() + 1);
}
#endif

#if 0
void MainWindow::openRecentMesh() {
    QAction *action = qobject_cast<QAction *>(sender());
    if (action)	loadModel(action->data().toString());
}

void MainWindow::openRecentProj() {
    QAction *action = qobject_cast<QAction *>(sender());
    if (action)	
        loadProject(action->data().toString());
}
#endif



#if 0
/*
 Save project. It saves the info of all the layers and the layer themselves. So
 */
void MainWindow::saveProject() {
    QFileDialog* saveDiag = new QFileDialog(this,tr("Save Project File"),lastUsedDirectory.path().append(""), tr("Starlab Project (*.starlab);;"));
    #if defined(Q_OS_MAC)
        saveDiag->setOption(QFileDialog::DontUseNativeDialog,true);
    #endif
    QCheckBox* saveAllFile = new QCheckBox(QString("Save All Files"),saveDiag);
    saveAllFile->setCheckState(Qt::Unchecked);
    QGridLayout* layout = (QGridLayout*) saveDiag->layout();
    layout->addWidget(saveAllFile,4,2);
    saveDiag->setAcceptMode(QFileDialog::AcceptSave);
    saveDiag->exec();
    
    QStringList files = saveDiag->selectedFiles();
    if (files.size() != 1)
        return;
    QString fileName = files[0];
    // this change of dir is needed for subsequent textures/materials ing
    QFileInfo fi(fileName);
    if (fi.isDir())
        return;
    if (fi.suffix().isEmpty()) {
        QRegExp reg("\\.\\w+");
        saveDiag->selectedNameFilter().indexOf(reg);
        QString ext = reg.cap();
        fileName.append(ext);
        fi.setFile(fileName);
    }
    QDir::setCurrent(fi.absoluteDir().absolutePath());

    document()->name=fileName;
    document()->path=fileName;
    
    if (fileName.isEmpty()) 
        return;
    else {
        //save path away so we can use it again
        QString path = fileName;
        path.truncate(path.lastIndexOf("/"));
        lastUsedDirectory.setPath(path);
    }
    
    if (QString(fi.suffix()).toLower() == "starlab")
        document()->writeToXML(document()->path);

    /// @todo 
    if (saveAllFile->isChecked()) {
        qDebug() << "saveAllFile not implemented";
#if 0
        for(int ii = 0; ii < meshDoc()->meshList.size(); ++ii) {
            MeshModel* mp = meshDoc()->meshList[ii];
            exportMesh(mp->fullName(),mp,true);
        }
#endif
    }
}
#endif


#if 0
// Opening files in a transparent form (IO plugins contribution is hidden to user)
bool MainWindow::loadModel(QString /*fileName*/, QString /*name*/) {
    /// Retrieve filenames to open from user
    QStringList fileNameList;
    { 
        if (fileName.isEmpty()){
            /// Generate a stringlist of "*.ext"
            QStringList extensions = pluginManager.knownExtensionsIO.keys();
            for(int i=0; i<extensions.size(); i++){
                QString& extension = extensions[i];
                extension.prepend("*.");
            }
            fileNameList = QFileDialog::getOpenFileNames(this,tr("Import Mesh"), lastUsedDirectory.path(), extensions.join(";;"));
        }
        else
            fileNameList.push_back(fileName);
    
        if (fileNameList.isEmpty()){
            return false;
        } else {
            //save path away so we can use it again
            QString path = fileNameList.first();
            path.truncate(path.lastIndexOf("/"));
            lastUsedDirectory.setPath(path);
        }
    }
    
    foreach(fileName,fileNameList){
        application()->loadModel(fileName, name);
        updateMenus();
        // updateModelToolBar();        
    }

    drawarea->resetView();
}
#endif

#ifdef OLDMESHLAB
bool MainWindow::exportMesh(QString /*fileName*/, Model* /*mod*/, bool /*saveAllPossibleAttributes*/) {
    /// @todo export mesh functionalities
    throw StarlabException("export mesh not implemented");
    
    bool ret = false;

    QStringList& suffixList = PM.outFilters;

    //QHash<QString, MeshIOInterface*> allKnownF ormats;
    QFileInfo fi(fileName);
    //PM.Formats( suffixList, allKnownFormats,PluginManager::EXPORT);
    //QString defaultExt = "*." + mod->suffixName().toLower();
    QString defaultExt = "*." + fi.suffix().toLower();
    if(defaultExt == "*.")
        defaultExt = "*.ply";
    if (mod == NULL)
        return false;
    mod->meshModified() = false;
    QString ff = mod->fullName();
    QFileDialog saveDialog(this,tr("Save Current Layer"), mod->fullName());
    saveDialog.setNameFilters(suffixList);
    saveDialog.setAcceptMode(QFileDialog::AcceptSave);
    QStringList matchingExtensions=suffixList.filter(defaultExt);
    if(!matchingExtensions.isEmpty())
        saveDialog.selectNameFilter(matchingExtensions.last());

    if (fileName.isEmpty()) {
        int dialogRet = saveDialog.exec();
        if(dialogRet==QDialog::Rejected	) return false;
        fileName=saveDialog.selectedFiles ().first();
        QFileInfo fni(fileName);
        if(fni.suffix().isEmpty()) {
            QString ext = saveDialog.selectedNameFilter();
            ext.chop(1);
            ext = ext.right(4);
            fileName = fileName + ext;
            qDebug("File without extension adding it by hand '%s'", qPrintable(fileName));
        }
    }

    QStringList fs = fileName.split(".");

    if(!fileName.isEmpty() && fs.size() < 2) {
        QMessageBox::warning(new QWidget(),"Save Error","You must specify file extension!!");
        return ret;
    }

    if (!fileName.isEmpty()) {
        //save path away so we can use it again
        QString path = fileName;
        path.truncate(path.lastIndexOf("/"));
        lastUsedDirectory.setPath(path);

        QString extension = fileName;
        extension.remove(0, fileName.lastIndexOf('.')+1);

        QStringListIterator itFilter(suffixList);

        MeshIOInterface *pCurrentIOPlugin = PM.allKnowOutputFormats[extension.toLower()];
        if (pCurrentIOPlugin == 0) {
            QMessageBox::warning(this, "Unknown type", "File extension not supported!");
            return false;
        }
        //MeshIOInterface* pCurrentIOPlugin = meshIOPlugins[idx-1];
        pCurrentIOPlugin->setLog(GLA()->log);

        int capability=0,defaultBits=0;
        pCurrentIOPlugin->GetExportMaskCapability(extension,capability,defaultBits);

        // optional saving parameters (like ascii/binary encoding)
        RichParameterSet savePar;

        pCurrentIOPlugin->initSaveParameter(extension,*(mod),savePar);

        SaveMaskExporterDialog maskDialog(new QWidget(),mod,capability,defaultBits,&savePar,this->GLA());
        if (!saveAllPossibleAttributes)
            maskDialog.exec();
        else {
            maskDialog.SlotSelectionAllButton();
            maskDialog.updateMask();
        }
        int mask = maskDialog.GetNewMask();
        if (!saveAllPossibleAttributes) {
            maskDialog.close();
            if(maskDialog.result() == QDialog::Rejected)
                return false;
        }
        if(mask == -1)
            return false;

        qApp->setOverrideCursor(QCursor(Qt::WaitCursor));
        qb->show();
        QTime tt;
        tt.start();
        ret = pCurrentIOPlugin->save(extension, fileName, *mod ,mask,savePar,QCallBack);
        qb->reset();
        GLA()->log->Logf(GLLogStream::SYSTEM,"Saved Mesh %s in %i msec",qPrintable(fileName),tt.elapsed());

        qApp->restoreOverrideCursor();
        mod->setFileName(fileName);
        QSettings settings;
        int savedMeshCounter=settings()->value("savedMeshCounter",0).toInt();
        settings()->setValue("savedMeshCounter",savedMeshCounter+1);
        GLA()->setWindowModified(false);

    }
    return ret;
}
#endif
