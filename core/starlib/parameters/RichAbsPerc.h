#pragma once
#include "RichParameter.h"

class RichAbsPerc : public RichParameter{ 
public:
    RichAbsPerc(QString name, float defaultValue=0, QString description=QString(), QString tooltip=QString())
        :RichParameter(name, description, tooltip){
        this->defaultValue = new AbsPercValue(defaultValue);
        this->val = new AbsPercValue(defaultValue);
    }
        
public:
    class AbsPercValue : public FloatValue {
    public:
        AbsPercValue(const float val) :FloatValue(val) {}
        inline float getAbsPerc() const { return getFloat(); }
        inline QString typeName() const { return QString("AbsPerc"); }
        inline bool isAbsPerc() const { return true; }
        ~AbsPercValue() {}
    };
};
