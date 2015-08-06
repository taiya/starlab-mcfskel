#include "Model.h"

#include <QTreeWidgetItem>  /// Plugins define items for layer tree menu
#include <QMenu>
#include <QToolBar>
#include <QAction>          /// Plugins define actions for toolbar buttons and such
#include <QFileInfo>        /// To automatically determine basename

#include "StarlabException.h"
#include "RenderPlugin.h"
#include "DecoratePlugin.h"

using namespace Starlab;

Model::Model(QString path, QString name){
    this->path = "";
    this->name = "";
    this->color = Qt::gray;
    this->isVisible = true;
    this->isModified = false;
    this->_renderer = NULL;
    this->path = path;
    this->name = name.isEmpty() ? QFileInfo(path).baseName() : name;    
}

Model::~Model(){
    if(_renderer) 
        delete _renderer;
}

void Model::decorateLayersWidgedItem(QTreeWidgetItem* parent){
    QTreeWidgetItem *fileItem = new QTreeWidgetItem();
    fileItem->setText(1, QString("Path"));    
    fileItem->setText(2, this->path);
    parent->addChild(fileItem);
    // updateColumnNumber(fileItem);
}

Renderer *Model::renderer(){
    return _renderer;
}

void Model::setRenderer(RenderPlugin* plugin){
    Q_ASSERT(plugin->isApplicable(this));
    if(_renderer != NULL){
        _renderer->finalize();
        _renderer->deleteLater();
        _renderer = NULL;
    }
    /// Get your own renderer instance
    _renderer = plugin->instance();   
    /// This deletes renderer upon model deletion
    _renderer->setParent(this);
    /// Record model and generating plugin inside renderer
    _renderer->_model = this;
    _renderer->_plugin = plugin;
    /// Now we are ready to initialize its parameters
    _renderer->initParameters();
    _renderer->init();
}

bool Model::hasDecoratePlugin(DecoratePlugin *plugin){
    return _decoratePlugins.contains(plugin);
}


void Model::removeDecoratePlugin(DecoratePlugin *plugin){
    _decoratePlugins.removeAll(plugin);
}

void Model::addDecoratePlugin(DecoratePlugin* plugin){
    plugin->_model = this;
    plugin->setParent(this);
    _decoratePlugins.append(plugin);
}

