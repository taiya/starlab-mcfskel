#pragma once
#include <QString>
#include <QDebug>
#include <QGridLayout>
#include <QWidget>
#include <QLabel>
#include <assert.h>
#include <stdexcept>
#include "starlib_global.h"

class RichParameterWidget;
class STARLIB_EXPORT RichParameter{
public: 
    class Value; ///< Forward declaration
public:
    /// Name of the parameter
    /// @note this is not the name that will appear in the GUI
    QString name;
    /// Parameter user specified Value
    Value* val;
    /// Default value (if not specified by the user)
    Value* defaultValue;
    /// Description of the parameter
    /// @todo rename into guiname, the string that will be put in the gui element
    QString description;
    /// Tooltip (mouse hovers) text
    QString tooltip;
    
    /// @obsolete replaced by description and tooltip
    // ParameterDecoration* pd;
    /// @obsolete
    // RichParameter(const QString nm,Value* v,ParameterDecoration* prdec){ assert(0); }
    /// @obsolete
    // virtual void accept(Visitor& v) = 0;
    /// @obsolete What components use comparator?
    // virtual bool operator==(const RichParameter& rp){ return (name==rp.name); }        

public:
    /// Constructor
    RichParameter(QString name="", QString description="", QString tooltip=""){ 
        this->name = name;
        this->description = description.isEmpty() ? name : description;
        this->tooltip = tooltip.isEmpty() ? name : tooltip;
    }
    virtual ~RichParameter(){}
 
    /// Returns a widget. If undefined by RichParameter adds nothing to the frame
    virtual RichParameterWidget* getWidget(QWidget* /*parent*/=0){
        throw std::logic_error("Widget for parameter not specified!");
    }
    
    /// Outputs a printable description of the parameter
    virtual QString toString(QString prefix=""){
        return prefix + this->name + " " + val->toString();
        // return prefix + this->name + " " + val->typeName() + " " + val->toString()  + " (default: " + defaultValue->toString() + ")";
    }
    
public:
    /// This is a pseudo-pure virtual. Its methods "assert" when something
    /// goes wrong...
    class Value {
    public:
        virtual bool             getBool() const {assert(0);return bool();}
        virtual int              getInt() const {assert(0);return int();}
        virtual float            getFloat() const {assert(0);return float();}
        virtual QString          getString() const {assert(0);return QString();}
        virtual QColor           getColor() const {assert(0);return QColor();}
        virtual float            getAbsPerc() const {assert(0);return float();}
        virtual int              getEnum() const {assert(0);return int();}
        virtual QString          getFileName() const {assert(0);return QString();}
        virtual QList<QString>   getStringSet() const {assert(0);return QList<QString>();}
        
        virtual bool isBool() const {return false;}
        virtual bool isInt() const {return false;}
        virtual bool isFloat() const {return false;}
        virtual bool isString() const {return false;}
        virtual bool isColor() const {return false;}
        virtual bool isAbsPerc() const {return false;}
        virtual bool isEnum() const {return false;}
        virtual bool isFileName() const {return false;}
        virtual bool isRichParameterSet() const{return false;}
        virtual bool isStringSet() const{return false;}
        
        virtual QString toString() { return "Value::toString() not implemented for this type"; }
        virtual QString typeName() const { assert(0); return QString(); }
        virtual void set(const Value& /*p*/) { assert(0); }
        virtual ~Value() {}
    };
};
