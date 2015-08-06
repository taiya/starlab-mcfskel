#include "ParametersFrame.h"
#include <QGridLayout>

void ParametersFrame::load(RichParameterSet* parameters){
    /// Stores parameters so we can later update them
    this->parameters = parameters;

    /// Schedule old ones for deletion
    {
        foreach(QWidget* widget, widgets)
            widget->deleteLater();
        widgets.clear();
    }
    
    /// @todo is this necessary or we can just do it in the constructor?
    {
        if(layout()) delete layout();
        QGridLayout * vLayout = new QGridLayout(this);
        vLayout->setAlignment(Qt::AlignTop);
    }
    
    /// Create widgets and insert them in the layout
    for(int i = 0; i < parameters->paramList.count(); i++) {
        RichParameter* fpi=parameters->paramList.at(i);
        RichParameterWidget* widget = fpi->getWidget(this);
        widgets.push_back(widget);
    }

    /// Once done filling, show form
    this->showNormal();
    // this->adjustSize(); unnecessary
}

void ParametersFrame::readValues(){
    foreach(RichParameterWidget* widget, widgets)
        widget->collectWidgetValue();
}
