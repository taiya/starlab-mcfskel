#include <QCoreApplication>
#include "CmdLineParser.h"
#include "StarlabApplication.h"
#include "PluginManager.h"
#include "interfaces/FilterPlugin.h"

using namespace Starlab;

QString safeCopyPath(QString path){
    QFileInfo fi(path);                 /// /tmp/archive.tar.gz
    QString dir = fi.absolutePath();    /// /tmp
    QString base = fi.baseName();       /// archive
    QString ext = fi.completeSuffix();  /// tar.gz
    
    QString newpath;
    int copynumber=1;
    do{
        newpath.sprintf("%s/%s_copy%d.%s",
                        qPrintable(dir),
                        qPrintable(base),
                        copynumber,
                        qPrintable(ext));
        copynumber++;
        // qDebug() << newpath;
    } while( QFileInfo(newpath).exists() );
    return newpath;
}

int main(int argc, char *argv[]){ 
    try{
        QCoreApplication* app = new QCoreApplication(argc, argv);
        QCoreApplication::setOrganizationName("Free Software Foundation");
        QCoreApplication::setApplicationName("starterm");
        QCoreApplication::setApplicationVersion(Application::version());
        CmdLineParser* parser = new CmdLineParser(argc,argv,app);
        Application* starlab = new Application();
        Document* document = starlab->document();
        
        /// Print a bunch of examples to command line
        if(parser->showExamples){
            QString exampleFilepath = starlab->starlabDirectory().absoluteFilePath("data/sphere.pts");
            qDebug("starterm --help");
            qDebug("starterm --list-filters");
            qDebug("starterm --filter=Normalize --save-overwrite %s", qPrintable(exampleFilepath));
            qDebug("starterm --list-filters %s", qPrintable(exampleFilepath));
            return 0;
        }
        
        /// Requested ALL filter list regardless of input
        if(parser->listFilters && parser->inputModels.isEmpty()){
            qDebug() << "Available filters (Model=Any): ";
            foreach(FilterPlugin* filter, starlab->applicableFilters())
                qDebug("--> %s \t %s", qPrintable(filter->name()), qPrintable(filter->description()));
            return 0;
        }
    
        /// Load models in the document
        if(!parser->inputModels.isEmpty()){
            qDebug() << "Loading models into the document";
            foreach(QString path, parser->inputModels)
                starlab->load(path);
            foreach(Model* model, document->models())
                qDebug("--> '%s' from '%s'",qPrintable(model->name),qPrintable(model->path));
        }
        
        /// Requested filters appropriate to given input
        if(parser->listFilters && !document->models().size()){
            Model* model = document->selectedModel();
            qDebug() << QString("Available filters (Model:%1):").arg( model->metaObject()->className() );
            foreach(FilterPlugin* filter, starlab->applicableFilters(model))
                qDebug("--> %s \t %s", qPrintable(filter->name()), qPrintable(filter->description()));
        }
        
        /// Executes the filter
        if(!parser->executeFilter.isEmpty() && document->models().size()>0 ){
            qDebug("Executing the filter '%s'", qPrintable(parser->executeFilter));
            foreach(Model* model, document->models()){
                starlab->executeFilter(model, parser->executeFilter);
                qDebug("--> Executed on '%s'",qPrintable(model->name));
            }
        }

        /// Saves results in a new file
        if(parser->saveCreatecopy && document->models().size()>0){
            qDebug() << "Saving filtered models (Safe Copy)";
            foreach(Model* model, document->models()){
                QString newPath = safeCopyPath(model->path);
                starlab->saveModel(model,newPath);
                qDebug("--> Saved '%s' at '%s'",qPrintable(model->name),qPrintable(newPath));
            }
        }
        
        /// Saves results by overwriting models
        if(parser->saveOverwrite && document->models().size()>0){
            qDebug("Saving filtered models (Overwriting)");
            foreach(Model* model, document->models()){
                starlab->saveModel(model,model->path);
                qDebug("--> Saved '%s' at '%s'",qPrintable(model->name),qPrintable(model->path));
            }
        }
    } 
    STARLAB_CATCH_BLOCK_NOGUI 
    return 0;                                
}
