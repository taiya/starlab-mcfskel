#include "project_io_starlab.h"
#include <QDomElement>
#include <QDir>
#include "StarlabApplication.h"

using namespace Starlab;

bool project_io_starlab::open(const QString& path, Application& application){
    /// Attempt open file resources
    QFile qf(path);
    QFileInfo qfInfo(path);
    QDir tmpDir = QDir::current();
    {
        QDir::setCurrent(qfInfo.absoluteDir().absolutePath());
        if( !qf.open(QIODevice::ReadOnly) )
            throw StarlabException("Cannot open project file");
        
        /// Attempts to open starlab project
        QDomDocument doc("StarLabProject");
        if(!doc.setContent( &qf ))
            throw StarlabException("Project is not a Starlab project");    
        
        /// Parse the XML
        QDomElement root = doc.documentElement();
        for(QDomNode node = root.firstChild(); !node.isNull(); node=node.nextSibling()){       
            if( node.nodeName()=="Layers" ){
                /// Read the models one by one
                for(QDomNode layerNode = node.firstChild(); !layerNode.isNull(); layerNode=layerNode.nextSibling()){
                    /// Extract elements
                    QString path=layerNode.attributes().namedItem("path").nodeValue();
                    QString label=layerNode.attributes().namedItem("label").nodeValue();
                    
                    /// Checks
                    if(path.isEmpty()) throw StarlabException("Model 'path' in project should not be empty");
                    if(label.isEmpty()) label = QFileInfo(path).baseName();
                    
                    /// Request a model to be opened
                    /// @todo right now the default opener is employed, in the project file we could encode
                    ///       which data structure it's supposed to be used
                    bool retstatus = application.loadModel(path);
                    if(retstatus==false)
                        qDebug() << "[WARNING] cannot open model" << path;
                }
            }
        }
    }
    QDir::setCurrent(tmpDir.absolutePath());
    qf.close();
        
    return true;
}

void project_io_starlab::save(const QString& /*path*/, Document& /*doc*/){
    throw StarlabException("Not implemented");
}

//QDomElement Model::toXML(QDomDocument &parent){
//    QDomElement elem = parent.createElement("Model");
//    elem.setAttribute("label",this->name);
//    elem.setAttribute("filename",this->path);
//    return elem;
//}

//void Document::writeToXML(QString filename){
//    QFileInfo fi(filename);
//    /// Cache current dir
//    QDir tmpDir = QDir::current();
//    QDir::setCurrent(fi.absoluteDir().absolutePath());
//    QDomDocument doc = this->toXML();
//    QFile file(filename);
//    file.open(QIODevice::WriteOnly);
//    QTextStream qstream(&file);
//    doc.save(qstream,1);
//    file.close();
//    /// Restore cached dir
//    QDir::setCurrent(tmpDir.absolutePath());
//}

/// This serializes the model to an XML description. Used by the core
/// to write XML project files. The default implementation doesn't 
/// encode what particular Model has been saved.
//QDomDocument Document::toXML(){
//    QDomDocument ddoc("StarLabProject");
//    QDomElement root = ddoc.createElement("StarLabProject");
//    {
//        QDomElement mgroot = ddoc.createElement("Layers");
//        {
//            foreach(Model* model, models)
//                mgroot.appendChild(model->toXML(ddoc));
//        }
//        root.appendChild(mgroot);                
//    }
//    ddoc.appendChild( root );        
//    return ddoc;
//}

//void Document::readFromXML(QString filename){

//}

Q_EXPORT_PLUGIN(project_io_starlab)
