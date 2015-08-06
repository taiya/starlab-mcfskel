#pragma once
#include "RichParameter.h"

class RichColor : public RichParameter{
public:
    RichColor(QString name, QColor defaultValue=Qt::black, QString description="", QString tooltip="")
        :RichParameter(name, description, tooltip){
        this->defaultValue = new Value(defaultValue);
        this->val = new Value(defaultValue);        
    }

    class Value : public RichParameter::Value {
    public:
        Value(QColor val) :pval(val) {}
        inline QColor getColor() const { return pval; }
        inline bool isColor() const { return true; }
        inline QString typeName() const { return QString("Color"); }
        inline void set(const RichParameter::Value& p) { pval = p.getColor(); }
        ~Value() {}
        QString toString() { 
            QString retval;
            QColor c = getColor();
            retval += "[";        
            retval += QString::number(c.red()) + " ";
            retval += QString::number(c.green()) + " ";
            retval += QString::number(c.blue());
            /// Only when color is not solid
            if(c.alpha()!=255) retval += " " + QString::number(c.alpha());
            retval += "]";        
            return retval;
        }
    private:
        QColor pval;
    };
};
