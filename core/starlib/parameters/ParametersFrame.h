#pragma once
#include <QFrame>
#include <QWidget>
#include <QKeyEvent>
#include "RichParameter.h"
#include "parameters/RichParameterSet.h"
#include "RichParameterWidget.h"
#include "starlib_global.h"

/// @todo My version of the old StdParFrame
class STARLIB_EXPORT ParametersFrame : public QFrame {
    Q_OBJECT

private:
    /// The parameters represented by this frame
    RichParameterSet* parameters;
    QList<RichParameterWidget*> widgets; 
    
public:
    ~ParametersFrame(){}
    ParametersFrame(QWidget* parent=NULL, Qt::WindowFlags f = 0) : QFrame(parent, f){
        parameters = NULL;
        if(layout()) delete layout();
        QGridLayout * vLayout = new QGridLayout(this);
        vLayout->setAlignment(Qt::AlignTop);
        // Show only when loading frame content
        this->hide();
        
        // when gui changes, read the values into the parameters
        connect(this,SIGNAL(parameterChanged()),SLOT(readValues()));
    }
    
    void load(RichParameterSet* parameters);
    void toggleHelp(){}
    
public slots:
    void readValues();
   
signals:
    void dynamicFloatChanged(int mask);
    void parameterChanged();
};
