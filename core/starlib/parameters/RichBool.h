#pragma once
#include <QCheckBox>
#include "RichParameterWidget.h"

#include "RichParameter.h"

class RichBool : public RichParameter{
public:
    RichBool(QString name, bool defaultValue=0, QString description=QString(), QString tooltip=QString())
        :RichParameter(name, description, tooltip){
        this->defaultValue = new Value(defaultValue);
        this->val = new Value(defaultValue);
    }

    virtual RichParameterWidget* getWidget(QWidget* parent){return new Widget(parent,this);}

public:    
    class Widget : public RichParameterWidget {
        QLabel* lab;
        QCheckBox* cb;
    public:
        ~Widget(){ delete lab; delete cb; }
        Widget(QWidget* parent,RichBool* rBool) : RichParameterWidget(parent,rBool) {
            cb = new QCheckBox("",parent);
            cb->setChecked(rp->val->getBool());
            lab = new QLabel(rBool->description, this);
            lab->setToolTip(rp->tooltip);
            gridLay->addWidget(lab,row,0,Qt::AlignLeft);
            gridLay->addWidget(cb,row,1,Qt::AlignRight);
            connect(cb,SIGNAL(stateChanged(int)),parent,SIGNAL(parameterChanged()));
        }
        void collectWidgetValue(){ rp->val->set(Value(cb->isChecked())); }
        void resetWidgetValue(){ cb->setChecked(rp->defaultValue->getBool()); }
        void setWidgetValue(const Value& nv){ cb->setChecked(nv.getBool()); }
    };
    
public:
    /// @todo rename to RichBool::Value
    class Value : public RichParameter::Value {
    public:
        Value(const bool val) : pval(val) {}
        inline bool getBool() const { return pval; }
        inline bool isBool() const { return true; }
        inline QString typeName() const { return QString("Bool"); }
        inline void set(const RichParameter::Value& p) { pval  = p.getBool(); }
        QString toString() { return (this->getBool()?"true":"false"); }
    private:
        bool pval;
    };
};
