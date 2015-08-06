#include <QtGui>
#include <QDebug>

#include "layerDialog.h"
#include "Model.h"
#include "ui_layerDialog.h"

using namespace std;
using namespace Starlab;

/// Basic widget item of the layers interface for models
class LayersWidgetModelItem : public QTreeWidgetItem{
public:
    /// @todo change to Model const*
    Model& model;
public:
    LayersWidgetModelItem(Model& _model) : 
        QTreeWidgetItem(), model(_model){
        if( model.isVisible) setIcon(0,QIcon(":/images/layer_eye_open.png"));
        if(!model.isVisible) setIcon(0,QIcon(":/images/layer_eye_close.png"));
        QString modelname = model.name;
        if (model.isModified) modelname += " *";
        setText(1, modelname);
    }
};

LayerDialog::~LayerDialog(){ delete ui; }

LayerDialog::LayerDialog(MainWindow* mainWindow) : 
    QDockWidget(mainWindow)
{
    this->setWindowTitle("Document Layers");
    this->mainWindow=mainWindow;
    
    ui = new Ui::layerDialog();
    setWindowFlags( windowFlags() | Qt::WindowStaysOnTopHint | Qt::SubWindow );
    this-> QWidget::setAttribute( Qt::WA_MacAlwaysShowToolWindow);
    setVisible(false);
    LayerDialog::ui->setupUi(this);
    
    /// When document changes, update layer table
    connect(mainWindow->document(), SIGNAL(hasChanged()), this, SLOT(updateTable()));
    
    // The following connection is used to associate the click with the switch between raster and mesh view.
    connect(ui->modelTreeWidget, SIGNAL(itemClicked(QTreeWidgetItem * , int  )) , this, SLOT(modelItemClicked(QTreeWidgetItem * , int ) ) );
    connect(ui->modelTreeWidget, SIGNAL(itemExpanded(QTreeWidgetItem * )) ,       this, SLOT(adaptLayout(QTreeWidgetItem *)));
    connect(ui->modelTreeWidget, SIGNAL(itemCollapsed(QTreeWidgetItem * )) ,      this, SLOT(adaptLayout(QTreeWidgetItem *)));
    /// @todo buttons at the bottom of window
    // connect(ui->addButton,    SIGNAL(clicked()), mw, SLOT(loadModel()) );
    // connect(ui->deleteButton, SIGNAL(clicked()), mw, SLOT(deleteModel()) );

    /// Layer contextual menu
    {
        this->setContextMenuPolicy(Qt::CustomContextMenu);
        ui->modelTreeWidget->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(ui->modelTreeWidget, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(showContextMenu(const QPoint&)));
    } 
}

void LayerDialog::modelItemClicked(QTreeWidgetItem* item , int column_number){
    LayersWidgetModelItem* mitem = dynamic_cast<LayersWidgetModelItem*>(item); 
    if(mitem){
        mainWindow->document()->pushBusy();
            /// Clicked on the eye, toggle visibility
            if( column_number==0 )
                mitem->model.isVisible = !mitem->model.isVisible;
            /// A click on any column makes a selection
            if( column_number>0  ){
                mainWindow->document()->setSelectedModel( &( mitem->model ) );
            }
            updateTable();
        mainWindow->document()->popBusy();
    }
}

void LayerDialog::showEvent (QShowEvent* /* event*/){
    updateTable();
}

#if 0
void LayerDialog::showLayerMenu(){
    foreach (QWidget *widget, QApplication::topLevelWidgets()) {
        MainWindow* mainwindow = dynamic_cast<MainWindow*>(widget);
        if (mainwindow){
            mainwindow->layerMenu()->popup(ui->menuButton->mapToGlobal(QPoint(10,10)));
            return;
        }
    }
}
#endif

void LayerDialog::showContextMenu(const QPoint& /*pos*/){
    qDebug() << "LayerDialog::showContextMenu() not implemented";
    /// SEE: mainwindow->layerMenu()->popup(ui->modelTreeWidget->mapToGlobal(pos));
}

void LayerDialog::updateTable(){
    // qDebug() << __FUNCTION__ << __LINE__ << __FILE__;
    
    //TODO:Check if the current viewer is a GLArea
    if(!isVisible()) return;
    Document* md = mainWindow->document();
    
    /// Setup the layer table
    ui->modelTreeWidget->clear();
    // The fourth column is fake to avoid that the last column
    // stretches indefinitively... how to avoid?    
    ui->modelTreeWidget->setColumnCount(3);
    ui->modelTreeWidget->setColumnWidth(0,40);
    ui->modelTreeWidget->setColumnWidth(1,20);
    // don't show the table header cells
    ui->modelTreeWidget->header()->hide(); 
    
    // Delegate the particular Model the 
    // task to specify a layer widget item
    foreach(Model* mi, md->models()){
        // Ask model to generate an item
        QTreeWidgetItem* item = mi->getLayersWidgetItem();
        if(item==NULL) item = new LayersWidgetModelItem(*mi);
        mi->decorateLayersWidgedItem(item);
                
        // Change color if currently selected
        if(mi == mainWindow->document()->selectedModel()){
            item->setBackground(1,QBrush(Qt::yellow));
            item->setForeground(1,QBrush(Qt::blue));
        }
        // Add it to the tree
        ui->modelTreeWidget->addTopLevelItem(item);
    }
    adaptLayout(NULL);
}

void LayerDialog::adaptLayout(QTreeWidgetItem* /*item*/){
    for(int i=0; i< ui->modelTreeWidget->columnCount(); i++)
        ui->modelTreeWidget->resizeColumnToContents(i);
}

void LayerDialog::on_moveModelUp_released(){
    document()->raise_layer( document()->selectedModel() );
}

void LayerDialog::on_moveModelDown_released(){
    document()->lower_layer( document()->selectedModel() );
}

void LayerDialog::on_deleteModel_released(){
    Document* doc = mainWindow->document();
    Model* model = doc->selectedModel();
    doc->deleteModel(model);
    updateTable();
}
