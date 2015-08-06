#pragma once
#include "RichParameter.h"

class RichEnum : public RichParameter{ 
public:
    RichEnum(QString name, int defaultValue=0, QString description=QString(), QString tooltip=QString())
        :RichParameter(name, description, tooltip){
        this->defaultValue = new EnumValue(defaultValue);
        this->val = new EnumValue(defaultValue);
    }
    
public:
    class EnumValue : public IntValue {
    public:
        EnumValue(const int val) :IntValue(val) {}
        inline int getEnum() const { return getInt(); }
        inline bool isEnum() const { return true; }
        inline QString typeName() const { return QString("Enum"); }
        ~EnumValue() {}
    };
};
