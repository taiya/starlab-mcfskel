#pragma once

#include <QtGui>
#include <QDockWidget>
#include "Starlab.h"
#include "Model.h"
#include "RichParameterSet.h"
#include "StarlabMainWindow.h"

class GLArea;
class QTreeWidget;
class GLLogStream;

#include <QDialog>
namespace Ui{ class layerDialog; }

/// @todo consider transforming this into yet another "Edit" plugin
class LayerDialog : public QDockWidget{
    Q_OBJECT
private:
    Ui::layerDialog* ui;
    Starlab::MainWindow* mainWindow;
    StarlabDocument* document(){ return mainWindow->document(); }
public:
    ~LayerDialog();
    LayerDialog(Starlab::MainWindow* mainwindow = 0);
    void updateDecoratorParsView();
    
public slots:
    /// Fills/Updates the layer table with content
    void updateTable();
    /// Restores the size of columns according to their content 
    /// @todo consider removing the parameter for this reason it might end up being NULL at times.
    void adaptLayout(QTreeWidgetItem * item);
    
    void modelItemClicked(QTreeWidgetItem * , int );
    void showEvent( QShowEvent * event );
    void showContextMenu(const QPoint& pos);

/// @{ slots for buttons at bottom of layer dialog
private slots:
    void on_moveModelUp_released();
    void on_moveModelDown_released();
    void on_deleteModel_released();
/// @}
};
