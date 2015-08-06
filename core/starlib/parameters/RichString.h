#pragma once
#include "RichParameter.h"
#include "RichParameterWidget.h"
#include "LineEditWidget.h"

class RichString : public RichParameter{
public:
    virtual RichParameterWidget* getWidget(QWidget* parent){ return new Widget(parent, this); }    

    RichString(QString name, QString defaultValue="", QString description="", QString tooltip="")
        :RichParameter(name, description, tooltip){
        this->defaultValue = new Value(defaultValue);
        this->val = new Value(defaultValue);        
    }
    
public:
    class Widget : public LineEditWidget{
    public:
        Widget(QWidget* p,RichString* rpar) : LineEditWidget(p,rpar) {
            this->lned->setAlignment(Qt::AlignRight);   
            lned->setText(rp->val->getString());
        }
        void collectWidgetValue(){
            rp->val->set(Value(lned->text()));
        }
        void resetWidgetValue(){
            lned->setText(rp->defaultValue->getString());
        }
        void setWidgetValue(const Value& nv){
            lned->setText(nv.getString());
        }
    };
    
    class Value : public RichParameter::Value {
    public:
        Value(const QString& val) :pval(val) {}
        inline QString getString() const { return pval; }
        inline bool isString() const { return true; }
        inline QString typeName() const { return QString("String"); }
        inline void	set(const RichParameter::Value& p) { pval = p.getString(); }
        QString toString() { return getString(); }
        ~Value() {}
    private:
        QString pval;
    };
};
