#pragma once
#include <QWidget>
#include <QLabel>
#include <QGridLayout>
#include <QLineEdit>
#include "starlib_global.h"
#include "RichParameter.h"

class STARLIB_EXPORT RichParameterWidget : public QWidget {
    Q_OBJECT
protected:
    /// Parameter we are creating a widget for
    RichParameter* rp;
    /// @todo why do we need to store this?
    QLabel* helpLab;    
    
public:
    ~RichParameterWidget(){ delete helpLab; }
    RichParameterWidget(QWidget* p, RichParameter* rpar) : QWidget(p){
        this->rp = rpar;
        
        assert(p!=NULL);
        if (rp!= NULL) {
            helpLab = new QLabel("<small>"+rpar->tooltip +"</small>",p);
            helpLab->setTextFormat(Qt::RichText);
            helpLab->setWordWrap(true);
            helpLab->setVisible(false);
            helpLab->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
            helpLab->setMinimumWidth(250);
            helpLab->setMaximumWidth(QWIDGETSIZE_MAX);
            gridLay = qobject_cast<QGridLayout*>(p->layout());
            assert(gridLay != 0);
    
            row = gridLay->rowCount();
            //WARNING!!!!!!!!!!!!!!!!!! HORRIBLE PATCH FOR THE BOOL WIDGET PROBLEM
            if ((row == 1) && (rpar->val->isBool())) {
                QLabel* lb = new QLabel("",p);
                gridLay->addWidget(lb);
                gridLay->addWidget(helpLab,row+1,3,1,1,Qt::AlignTop);
            }
            ///////////////////////////////////////////////////////////////////////
            else
                gridLay->addWidget(helpLab,row,3,1,1,Qt::AlignTop);
        }
    }

    /// Restore gui values with defaults stored in RichParameter
    virtual void resetWidgetValue() = 0;
    /// Reads the value from the gui and stores it in "this->rp"
    virtual void collectWidgetValue() = 0;
    /// @todo What is this?
    virtual void setWidgetValue(const RichParameter::Value& nv) = 0;
    
signals:
    void parameterChanged();

protected:
    void InitRichParameter(RichParameter* rpar);
    int row;
    QGridLayout* gridLay;
};
