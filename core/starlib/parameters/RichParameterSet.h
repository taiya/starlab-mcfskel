#pragma once
#include <QList> // Used to stores the set of parameters
#include <QObject>

#include "starlib_global.h"
#include "RichParameter.h"
#include "RichFloat.h"
#include "RichBool.h"
#include "RichInt.h"
#include "RichString.h"
#include "RichColor.h"
#include "RichStringSet.h"

class STARLIB_EXPORT RichParameterSet : public QObject, public RichParameter{
    Q_OBJECT
    
private:
    Q_DISABLE_COPY(RichParameterSet)
    
/// @{ constructors/destructors
public:
    RichParameterSet(QObject* parent=NULL) : QObject(parent){}
    RichParameterSet(QString _name, QObject* parent=NULL) : QObject(parent), RichParameter(_name){}
    ~RichParameterSet();
/// @}

public:    
    QList<RichParameter*> paramList;
    bool isEmpty() const;
    RichParameter* findParameter(QString name) const;
    bool hasParameter(QString name) const;

    /// Generates a string representation of the instance
    QString toString(QString prefix=QString());
    
    bool operator==(const RichParameterSet& rps);

    RichParameterSet& addParam(RichParameter* pd);

    //remove a parameter from the set by name
    RichParameterSet& removeParameter(QString name);

    void clear();

    /// General setter (require you to instantiate a Value type)
    void setValue(const QString name,const Value& val);
    
    
    bool getBool(QString name) const;
    void setValue(QString name, bool newval);
    
    int	getInt(QString name) const;
    void setValue(QString name, int newval);
    
    float getFloat(QString name) const;
    void setValue(QString name, float newval);
        
    QString getString(QString name) const;
    void setValue(QString name, const QString& newval);
    
    QColor getColor(QString name) const;
    void setValue(QString name, const QColor& newval);

/// Reimplement these if needed
#ifdef OLDMESHLAB    
    float getAbsPerc(QString name) const;
    int	getEnum(QString name) const;
    QList<float> getFloatList(QString name) const;
    float getDynamicFloat(QString name) const;
    QString getOpenFileName(QString name) const;
    QString getSaveFileName(QString name) const;
#endif
    
#ifdef OLDMESHLAB
    vcg::Color4b getColor4b(QString name) const;
    vcg::Matrix44f getMatrix44(QString name) const;
    vcg::Point3f getPoint3f(QString name) const;
    vcg::Shotf getShotf(QString name) const;
#endif    
    
#ifdef OLDMESHLAB
    /// From RichParameter
    virtual bool operator==(const RichParameter& rp){
        bool retval=true;
        retval &= (name == rp.name);
        retval &= rp.val->isRichParameterSet();
        /// @todo perform recursive check
        assert(0);
    }
#endif
};
