#pragma once
#include "RichParameter.h"
#include "LineEditWidget.h"


class RichFloat : public RichParameter{
public:
    virtual RichParameterWidget* getWidget(QWidget* parent){ return new RichFloat::Widget(parent,this); }
    
    RichFloat(QString name, float defaultValue=0, QString description="", QString tooltip="")
        :RichParameter(name, description, tooltip){
        this->defaultValue = new Value(defaultValue);
        this->val = new Value(defaultValue);        
    }

public:
    /// @todo rename to RichFloat::Value
    class Value : public RichParameter::Value {
    public:
        Value(const float val){ this->pval = val; }
        inline float getFloat() const { return pval; }
        inline bool isFloat() const { return true; }
        inline QString typeName() const { return QString("Float"); }
        inline void set(const RichParameter::Value& p) { pval = p.getFloat(); }
        QString toString() { return QString::number(getFloat()); }
    private:
        float pval;
    };
    
    class Widget : public LineEditWidget {
    public:
        Widget(QWidget* p,RichFloat* rpar) : LineEditWidget(p,rpar) {
            this->lned->setAlignment(Qt::AlignRight);
            lned->setText(QString::number(rp->val->getFloat(),'g',3));
        }
        void collectWidgetValue(){ 
            rp->val->set(RichFloat::Value(lned->text().toFloat())); 
        }
        void resetWidgetValue(){
            lned->setText(QString::number(rp->defaultValue->getFloat(),'g',3));            
        }
        void setWidgetValue(const RichParameter::Value& nv){
            lned->setText(QString::number(nv.getFloat(),'g',3));                        
        }
    };
};
