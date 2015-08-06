#pragma once
#include <QApplication>
#include "RichParameterWidget.h"

class LineEditWidget : public RichParameterWidget {
protected:
    QLabel* lab;
    QLineEdit* lned;

public:
    ~LineEditWidget(){ delete lned; delete lab; }
    LineEditWidget(QWidget* p,RichParameter* rpar) : RichParameterWidget(p,rpar) {
        lab = new QLabel(rp->description,this);
        lned = new QLineEdit(this);
        lned->setMaximumWidth(3000);    
    
        /// Autodetermine text field width
        int width = 15*QFontMetrics(QApplication::font()).width('0');
        gridLay->setColumnMinimumWidth(1, width);
        
        lab->setToolTip(rp->tooltip);
        gridLay->addWidget(lab,row,0,Qt::AlignLeft);
        gridLay->addWidget(lned,row,1,Qt::AlignRight);
        connect(lned,SIGNAL(editingFinished()),p,SIGNAL(parameterChanged()));
    }
    virtual void collectWidgetValue() = 0;
    virtual void resetWidgetValue() = 0;
    virtual void setWidgetValue(const RichParameter::Value& nv) = 0;
};
