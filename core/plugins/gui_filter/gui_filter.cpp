#include "gui_filter.h"
Q_EXPORT_PLUGIN(gui_filter)

#include <QToolTip>
#include <QToolBar>
#include <QMessageBox>

#include "OSQuery.h"
#include "FilterDockWidget.h"
#include "StarlabMainWindow.h"
#include "parameters/RichParameterSet.h"
#include "interfaces/FilterPlugin.h"
#include <QElapsedTimer>

void gui_filter::load(){
    /// Fill the menu with plugin names and make connections
    foreach(FilterPlugin* plugin, pluginManager()->filterPlugins()){
        QAction* action = plugin->action();
        mainWindow()->filterMenu->addAction(action);
        
        // Action refers to this filter, so we can retrieve it later
        // QAction* action = new QAction(plugin->name(),plugin);
        
        
        /// Does the filter have an icon? Add it to the toolbar
        /// @todo add functionality to ModelFilter
        if(!action->icon().isNull())
            mainWindow()->filterToolbar->addAction(action);

        /// Connect after it has been added        
        connect(action,SIGNAL(triggered()),this,SLOT(startFilter()));
    }
}

// It is splitted in two functions
// - startFilter that setup the dialogs and asks for parameters
// - executeFilter callback invoked when the params have been set up.
void gui_filter::startFilter() {
    try
    {
        // In order to avoid that a filter changes something assumed by the current editing tool,
        // before actually starting the filter we close the current editing tool (if any).
        /// @todo 
        // mainWindow()->endEdit(); 
        // mainWindow()->updateMenus();
    
        QAction* action = qobject_cast<QAction*>(sender());
        FilterPlugin* iFilter = qobject_cast<FilterPlugin*>(action->parent());
        if(!iFilter) return;
        
        /// Even though it's on the stack we associate it with the widget 
        /// in such a way that memory will get deleted when widget goes out 
        /// of scope
        RichParameterSet* parameters = new RichParameterSet();
        iFilter->initParameters(parameters);
        int needUserInput = !parameters->isEmpty();
        
        /// I do not need the user input, just run it
        switch(needUserInput){
            case false:
                execute(iFilter, parameters);
                break;
            case true:
                FilterDockWidget* widget = new FilterDockWidget(iFilter,parameters,mainWindow());
                connect(widget,SIGNAL(execute(FilterPlugin*,RichParameterSet*)), this,SLOT(execute(FilterPlugin*,RichParameterSet*)));            
                mainWindow()->addDockWidget(Qt::RightDockWidgetArea, widget);
                widget->show();
                break;
        }
    } 
    STARLAB_CATCH_BLOCK
}

void gui_filter::execute(FilterPlugin* iFilter, RichParameterSet* parameters) {
    if(!iFilter->isApplicable(document()->selectedModel())) 
        throw StarlabException("Filter is not applicable");

    /// @todo save the current filter and its parameters in the history
    // meshDoc()->filterHistory.actionList.append(qMakePair(iFilter->name(),params));
    
    /// @todo re-link the progress bar
    // qb->reset();    
    
    qApp->setOverrideCursor(QCursor(Qt::WaitCursor));
    document()->pushBusy();
        try {
            QElapsedTimer t;
            t.start();

            iFilter->applyFilter(parameters);

            mainWindow()->setStatusBarMessage("Filter '"+ iFilter->name() +"' Executed " + QString("(%1ms).").arg(t.elapsed()),5000);
        } STARLAB_CATCH_BLOCK
    document()->popBusy();
    qApp->restoreOverrideCursor();
    mainWindow()->closeProgressBar();
}

