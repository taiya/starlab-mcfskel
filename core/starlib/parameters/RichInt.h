#pragma once
#include "RichParameter.h"
#include "LineEditWidget.h"
class RichInt : public RichParameter{
public:
    virtual RichParameterWidget* getWidget(QWidget* parent){ return new Widget(parent, this); }    

    RichInt(QString name, int defaultValue=0, QString description=QString(), QString tooltip=QString())
        :RichParameter(name, description, tooltip){
        this->val = new Value(defaultValue);
        this->defaultValue = new Value(defaultValue);
    }
        
    class Widget : public LineEditWidget {
    public:
        Widget(QWidget* p,RichInt* rpar):LineEditWidget(p,rpar) {
            this->lned->setAlignment(Qt::AlignRight);
            lned->setText(QString::number(rp->val->getInt()));
        }
        void collectWidgetValue(){
            rp->val->set(Value(lned->text().toInt()));            
        }
        void resetWidgetValue(){    
            lned->setText(QString::number(rp->defaultValue->getInt()));
        }
        void setWidgetValue(const Value& nv){
            lned->setText(QString::number(nv.getInt()));
        }
    };
    
    /// @todo rename to RichInt::Value
    class Value : public RichParameter::Value {
    private:
        int pval;
    public:
        Value(const int val) : pval(val) {}
        inline int	getInt() const { return pval; }
        inline bool isInt() const { return true; }
        inline QString typeName() const { return QString("Int"); }
        inline void	set(const RichParameter::Value& p) { pval = p.getInt(); }
        QString toString() { return QString::number(getInt()); }
    };
};
