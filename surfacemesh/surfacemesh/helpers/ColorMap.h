/// To use must have defined these types:
/// ==> Scalar
/// ==> Color

#pragma once
#include <QColor>
#include <cmath>

#ifdef WIN32
    namespace std{  bool isnan(double x){ return _isnan(x); }
                    bool isinf(double x){ return _finite(x); } }
#endif

// Temporary solution
static inline Color toColor(QColor c){
    return Color(c.redF(), c.greenF(), c.blueF());
}

class ColorMap{
protected:
    Scalar min;
    Scalar max;
    bool validRange;
public:
    virtual void init(Scalar min, Scalar max){
        this->min = min;
        this->max = max;
        validRange = ( fabs(max-min) > std::numeric_limits<Scalar>::min() );
    }
    virtual Color color(Scalar /*val*/) = 0;
};

class SignedColorMap : public ColorMap{
protected:
    Scalar largestBound;
    QColor negative;
    QColor zero;
    QColor positive;
    QColor invalid;
    
public:
    SignedColorMap(QColor negative=Qt::red, QColor zero=Qt::yellow, QColor positive=Qt::green, QColor invalid=Qt::black){
        this->negative = negative.toHsv();
        this->zero = zero.toHsv();
        this->positive = positive.toHsv();
        this->invalid = invalid.toHsv();        
    }
    virtual void init(Scalar min, Scalar max){
        ColorMap::init(min,max);        
        largestBound = qMax(qAbs(min),qAbs(max));
    }
    
    /// @todo maybe create a lookup table version?
    Color color(Scalar val){
        if(!validRange) return Color(toColor(invalid.toRgb()));
        if(std::isnan(val)) return Color(toColor(invalid.toRgb()));
        Scalar alpha = qBound( -1.0, val/largestBound, +1.0 ); // [-1,1]
        QColor retval;
        Scalar h,s,v;
        
        /// Interpolate differently on the sides
        if(alpha>=0){
            h = (1-alpha)*zero.hueF()        + (alpha)*positive.hueF();
            s = (1-alpha)*zero.saturationF() + (alpha)*positive.saturationF();
            v = (1-alpha)*zero.valueF()      + (alpha)*positive.valueF();
        }
        if(alpha<0){
            alpha = -alpha;
            h = (1-alpha)*zero.hueF()        + (alpha)*negative.hueF();
            s = (1-alpha)*zero.saturationF() + (alpha)*negative.saturationF();
            v = (1-alpha)*zero.valueF()      + (alpha)*negative.valueF();            
        }
        // qDebug() << h << s << v;
        retval.setHsvF(h,s,v);
        return Color(toColor(retval.toRgb()));
    }    
};

static SignedColorMap signedColorMap;

class UnsignedColorMap : public ColorMap{
private:
    QColor zero;
    QColor positive;
    QColor invalid;
public:
    UnsignedColorMap(){
        zero = Qt::white;
        positive = Qt::green;
        invalid = Qt::black;
    }
    virtual Color color(Scalar val){
        if(!validRange) return Color(toColor(invalid));
        if(std::isnan(val)) return Color(toColor(invalid)); //RED
        float alpha = qBound( 0.0, (val-min) / (max-min), 1.0 );
        
        Q_ASSERT(alpha>=0 && alpha<=1);
        /// HSV INTERPOLATE
#if 0
        Scalar h = (1-alpha)*zero.hueF()        + (alpha)*positive.hueF();
        Scalar s = (1-alpha)*zero.saturationF() + (alpha)*positive.saturationF();
        Scalar v = (1-alpha)*zero.valueF()      + (alpha)*positive.valueF();   
        QColor retval;
        retval.setHsvF(h,s,v);
        return Color(retval.toRgb());
#else
        Scalar r = (1-alpha)*zero.redF()   + (alpha)*positive.redF();
        Scalar g = (1-alpha)*zero.greenF() + (alpha)*positive.greenF();
        Scalar b = (1-alpha)*zero.blueF()  + (alpha)*positive.blueF();   
        return Color(r,g,b);
#endif
    }
};

static UnsignedColorMap unsignedColorMap;

