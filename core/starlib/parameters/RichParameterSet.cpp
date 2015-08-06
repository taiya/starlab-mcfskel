#include <QDebug>
#include "parameters/RichParameterSet.h"
 
// Very similar to the findParameter but this one does not print out debugstuff.
bool RichParameterSet::hasParameter(QString name) const {
    QList<RichParameter*>::const_iterator fpli;
    for(fpli=paramList.begin(); fpli!=paramList.end(); ++fpli) {
        if((*fpli != NULL) && (*fpli)->name==name)
            return true;
    }
    return false;
}
// You should never use this one to know if a given parameter is present.
RichParameter* RichParameterSet::findParameter(QString name) const {
    QList<RichParameter*>::const_iterator fpli;
    for(fpli=paramList.begin(); fpli!=paramList.end(); ++fpli) {
        if((*fpli != NULL) && (*fpli)->name==name)
            return *fpli;
    }
    qWarning("Warning: Unable to find a parameter with name '%s' returning default value",qPrintable(name));
    return 0;
}

RichParameterSet& RichParameterSet::removeParameter(QString name) {
    paramList.removeAll(findParameter(name));
    return (*this);
}

RichParameterSet& RichParameterSet::addParam(RichParameter* pd ) {
    assert(!hasParameter(pd->name));
    paramList.push_back(pd);
    return (*this);
}

//--------------------------------------
void RichParameterSet::setValue(QString name,const Value& newval) {
    RichParameter* par = findParameter(name);
    assert( par->val->typeName() == newval.typeName() );
    par->val->set(newval);
}


//- All the get<TYPE> are very similar. Nothing interesting here.

bool RichParameterSet::getBool(QString name) const {
    RichParameter* par = findParameter(name);
    return par? par->val->getBool():false;
}
int RichParameterSet::getInt(QString name)      const {
    RichParameter* par = findParameter(name);
    return par? par->val->getInt():0;
}
float RichParameterSet::getFloat(QString name)    const {
    RichParameter* par = findParameter(name);
    return par? par->val->getFloat():0.0f;
}
QColor RichParameterSet::getColor(QString name)    const {
    RichParameter* par = findParameter(name);
    return par? par->val->getColor():QColor();
}
QString RichParameterSet::getString(QString name)   const {
    RichParameter* par = findParameter(name);
    return par? par->val->getString():"";
}

void RichParameterSet::setValue(QString name, bool newval){
    findParameter(name)->val->set(RichBool::Value(newval));
}

void RichParameterSet::setValue(QString name, int newval){
    findParameter(name)->val->set(RichInt::Value(newval));
}

void RichParameterSet::setValue(QString name, float newval){
    findParameter(name)->val->set(RichFloat::Value(newval));
}

void RichParameterSet::setValue(QString name, const QString& newval){
    findParameter(name)->val->set(RichString::Value(newval));
}

void RichParameterSet::setValue(QString name, const QColor& newval){
    findParameter(name)->val->set(RichColor::Value(newval));
}

#ifdef OLDMESHLAB
float RichParameterSet::getAbsPerc(QString name)  const {
    RichParameter* par = findParameter(name);
    return par? par->val->getAbsPerc():0.0f;
}
int RichParameterSet::getEnum(QString name) const {
    RichParameter* par = findParameter(name);
    return par? par->val->getEnum():0;
}
QString RichParameterSet::getOpenFileName(QString name) const {
    RichParameter* par = findParameter(name);
    return par? par->val->getFileName():"";
}
QString RichParameterSet::getSaveFileName(QString name) const {
    RichParameter* par = findParameter(name);
    return par? par->val->getFileName():"";
}
#endif

/// LEGACY
#if 0
/// @todo Implement (or get rid of it if unnecessary)
RichParameterSet& RichParameterSet::operator=( const RichParameterSet& /*rps*/ ) {
    assert(0); 
    return *this;
}

/// @todo Implement (or get rid of it if unnecessary)
RichParameterSet::RichParameterSet( const RichParameterSet& rps ) :
    RichParameter(rps.name, rps.description, rps.tooltip){
    assert(0); 
}

/// @todo Implement (or get rid of it if unnecessary)
RichParameterSet& RichParameterSet::join( const RichParameterSet& /*rps*/ ) {
    assert(0);
    return *this;
}
#endif

bool RichParameterSet::isEmpty() const {
    return paramList.isEmpty();
}

void RichParameterSet::clear() {
    paramList.clear();
}

RichParameterSet::~RichParameterSet() {
    // qDebug() << "~RichParameterSet()";
    for(int ii = 0; ii < paramList.size(); ++ii)
        delete paramList.at(ii);
    paramList.clear();
}

QString RichParameterSet::toString(QString prefix) {
    QString retval;
    // retval.append(prefix + this->name + "[RichParameterSet]:\n");
    if(!name.isEmpty()) retval.append(prefix + this->name + "\n");
    foreach(RichParameter* param, paramList) {
        /// @todo find out why there is a null param...
        if(param==NULL) continue;
        retval.append( prefix + param->toString("  ") + "\n" );
    }
    return retval;
}

#ifdef OLDMESHLAB
bool RichParameterSet::operator==( const RichParameterSet& rps ) {
    if (rps.paramList.size() != paramList.size())
        return false;

    bool iseq = true;
    int ii = 0;
    while((ii < rps.paramList.size()) && iseq) {
        if (!(*rps.paramList.at(ii) == *paramList.at(ii)))
            iseq = false;
        ++ii;
    }

    return iseq;
}

Matrix44f RichParameterSet::getMatrix44(QString name) const {
    RichParameter* par = findParameter(name);
    return par? par->val->getMatrix44f():Matrix44f();
}
Point3f RichParameterSet::getPoint3f(QString name)  const {
    RichParameter* par = findParameter(name);
    return par? par->val->getPoint3f():Point3f();
}
Shotf RichParameterSet::getShotf(QString name)    const {
    RichParameter* par = findParameter(name);
    return par? par->val->getShotf():Shotf();
}
QList<float> RichParameterSet::getFloatList(QString name)    const {
    RichParameter* par = findParameter(name);
    return par? par->val->getFloatList():QList<float>();
}
float RichParameterSet::getDynamicFloat(QString name) const {
    RichParameter* par = findParameter(name);
    return par? par->val->getDynamicFloat():0.0f;
}
#endif


#if 0 /// Not working...
#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug dbg, RichParameterSet& pars){
    dbg.nospace() << qPrintable( pars.toString() ); 
    return dbg;    
}
#endif
#endif 
