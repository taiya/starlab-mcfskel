#pragma once


//=============================================================== INCLUDES ====

#include <iostream>
//#include <assert.h>
#include <math.h>
#include <limits>
#include <cstring>

#ifdef QT_CORE_LIB
    #include <QColor>
    #include <QDebug>
    #include <QVector3D>
#endif

#ifdef WIN32
    #undef min
    #undef max
#endif

namespace CurveskelTypes{

/// \addtogroup geometry geometry
/// @{

//======================================================= CLASS DEFINITION ====

/// A vector class.
template <typename Scalar, int N>
class SkelVector
{
public:
    /// Qt Special stuff
#ifdef QT_CORE_LIB    
    SkelVector(QColor color){
        Q_ASSERT(dim()>=3);
        if(dim()==3){
            data_[0] = color.redF();
            data_[1] = color.greenF();
            data_[2] = color.blueF();
        }
    }
    SkelVector(const QVector3D& rhs){
        Q_ASSERT(dim()>=3);
        data_[0]=rhs.x();
        data_[1]=rhs.y();
        data_[2]=rhs.z();
    }
    
    operator QColor(){
        Q_ASSERT(dim()>=3);
        QColor c;
        c.setRedF(data_[0]);
        c.setGreenF(data_[1]);
        c.setBlueF(data_[2]);
        return c;
    }
    
    operator QVector3D(){ 
        return QVector3D(x(),y(),z()); 
    }
#endif
    
public:
    //--- class info ---
    
    ///the type of the Scalar used in this template
    typedef Scalar value_type;
    
    /// returns dimension N of the vector
    static int dim()
    {
        return N;
    }
    
    
    
    //--- constructors ---
    
    /// default constructor creates uninitialized values.
    SkelVector() {}
    
    /// construct from scalar s (fills all components with s)
    SkelVector(const Scalar s)
    {
        for (int i=0; i<N; i++) data_[i] = s;
    }
    
    
    /** constructor taking 2 explicit Scalar values; only valid for
     2D vectors. */
    SkelVector(const Scalar v0,const Scalar v1)
    {
        assert(N==2);
        data_[0] = v0; data_[1] = v1;
    }
    
    /** constructor taking 3 explicit Scalar values; only valid for
     3D vector. */
    SkelVector(const Scalar v0, const Scalar v1, const Scalar v2)
    {
        assert(N==3);
        data_[0]=v0; data_[1]=v1; data_[2]=v2;
    }
    
    /** constructor taking 4 explicit Scalar values; only valid for
     4D vectors. */
    SkelVector(const Scalar v0, const Scalar v1, const Scalar v2, const Scalar v3)
    {
        assert(N==4);
        data_[0]=v0; data_[1]=v1; data_[2]=v2; data_[3]=v3;
    }
    
    /// copy constructor
    template<typename otherScalarType>
    SkelVector(const SkelVector<otherScalarType,N> &o)
    {
        operator=(o);
    }
    
    
    
    
    //--- casts ---
    
    /// cast to Scalar array
    operator Scalar*()
    {
        return data_;
    }
    
    /// cast to const Scalar array
    operator const Scalar*() const
    {
        return data_;
    }
    
    
    
    
    //--- element access ---
    
    /// get i'th element read-write
    Scalar& operator[](int i)
    {
        assert(i>=0 && i<N);
        return data_[i];
    }
    
    /// get i'th element read-only
    const Scalar operator[](int i) const
    {
        assert(i>=0 && i<N);
        return data_[i];
    }
    
    Scalar& x()
    {
        assert(N > 0);
        return data_[0];
    }
    
    const Scalar& x() const
    {
        assert(N > 0);
        return data_[0];
    }
    
    Scalar& y()
    {
        assert(N > 1);
        return data_[1];
    }
    
    const Scalar& y() const
    {
        assert(N > 1);
        return data_[1];
    }
    
    Scalar& z()
    {
        assert(N > 2);
        return data_[2];
    }
    
    const Scalar& z() const
    {
        assert(N > 2);
        return data_[2];
    }
    
    
    
    //--- assignment, comparsion ---
    
    /// assign a scalar to all componenets
    SkelVector<Scalar,N>& operator=(const Scalar s)
    {
        for (int i=0; i<N; i++)
            data_[i] = s;
        return *this;
    }
    
    /// assignment from a vector of the same kind
    SkelVector<Scalar,N>& operator=(const SkelVector<Scalar,N> &o)
    {
        memcpy(data_, o.data_, N*sizeof(Scalar));
        return *this;
    }
    
    /// assignment from a vector of the different kind
    template<typename otherScalarType>
    SkelVector<Scalar,N>& operator=(const SkelVector<otherScalarType,N> &o)
    {
        for (int i=0; i<N; i++)
            data_[i] = Scalar(o[i]);
        return *this;
    }
    
    /// component-wise comparison
    bool operator==(const SkelVector<Scalar,N> &other) const
    {
        for (int i=0; i<N; i++)
            if (data_[i]!=other.data_[i])
                return false;
        return true;
    }
    
    /// component-wise comparison
    bool operator!=(const SkelVector<Scalar,N> &other) const
    {
        for (int i=0; i<N; i++)
            if (data_[i]!=other.data_[i])
                return true;
        return false;
    }
    
    
    
    
    //--- vector space operators ---
    
    /// component-wise self-multiplication with Scalar
    SkelVector<Scalar,N>& operator*=(const Scalar &s)
    {
        for (int i=0; i<N; i++)
            data_[i] *= s;
        return *this;
    }
    
    /// component-wise multiplication with Scalar
    SkelVector<Scalar,N> operator*(const Scalar &s) const
    {
        return SkelVector<Scalar,N>(*this) *= s;
    }
    
    /// component-wise self-division by Scalar
    const SkelVector<Scalar,N> &operator/=(const Scalar &s)
    {
        for (int i=0; i<N; i++)
            data_[i] /= s;
        return *this;
    }
    
    /// component-wise division by Scalar
    SkelVector<Scalar,N> operator/(const Scalar &s) const
    {
        return SkelVector<Scalar,N>(*this) /= s;
    }
    
    /// vector difference from this
    SkelVector<Scalar,N> &operator-=(const SkelVector<Scalar,N> &other)
    {
        for (int i=0; i<N; i++)
            data_[i] -= other.data_[i];
        return *this;
    }
    
    /// vector difference
    SkelVector<Scalar,N> operator-(const SkelVector<Scalar,N> &other) const
    {
        return SkelVector<Scalar,N>(*this) -= other;
    }
    
    /// vector self-addition
    SkelVector<Scalar,N> &operator+=(const SkelVector<Scalar,N> &other)
    {
        for (int i=0; i<N; i++)
            data_[i] += other.data_[i];
        return *this;
    }
    
    /// vector addition
    SkelVector<Scalar,N> operator+(const SkelVector<Scalar,N> &other) const
    {
        return SkelVector<Scalar,N>(*this) += other;
    }
    
    /// unary minus
    SkelVector<Scalar,N> operator-(void) const
    {
        SkelVector<Scalar,N> v;
        for (int i=0; i<N; i++)
            v.data_[i] = -data_[i];
        return v;
    }
    
    
    
    
    //--- norm stuff ---
    
    /// compute Euclidiean norm
    Scalar norm() const
    {
        return (Scalar)sqrt(sqrnorm());
    }
    
    /// compute squared norm
    Scalar sqrnorm() const
    {
        Scalar s(0.0);
        for (int i=0; i<N; i++)
            s += data_[i]*data_[i];
        return s;
    }
    
    /// normalize vector, return normalized vector
    SkelVector<Scalar,N>& normalize()
    {
        Scalar n = norm();
        if (n > std::numeric_limits<Scalar>::min())
            *this *= 1.0/n;
        return *this;
    }
    
    /// return normalized vector (does not change vector)
    const SkelVector<Scalar,N> normalized() const
    {
        return (*this) / norm();
    }
    
    
    //--- max and min for bounding box computations ---
    
    /// return vector with minimum of this and other in each component
    SkelVector<Scalar,N> minimize(const SkelVector<Scalar,N> &other)
    {
        for (int i = 0; i < N; i++)
            if (other[i] < data_[i])
                data_[i] = other[i];
        return *this;
    }
    
    /// return vector with maximum of this and other in each component
    SkelVector<Scalar,N> maximize(const SkelVector<Scalar,N> &other)
    {
        for (int i = 0; i < N; i++)
            if (other[i] > data_[i])
                data_[i] = other[i];
        return *this;
    }
    
    
    /// Read-only access to the data_ array
    const Scalar* data() const
    {
        return &data_[0];
    }
    
    
protected:
    
    /** The N values of type Scalar are the only data members
     of this class. This guarantees 100% compatibility with arrays of type
     Scalar and size N, allowing us to define the cast operators to and from
     arrays and array pointers */
    Scalar data_[N];
};



//== FUNCTIONS ================================================================


/// read the space-separated components of a vector from a stream
template <typename Scalar,int N>
inline std::istream& operator>>(std::istream& is, SkelVector<Scalar,N>& vec)
{
    for (int i=0; i<N; i++)
        is >> vec[i];
    return is;
}


/// output a vector by printing its space-separated compontens
template <typename Scalar,int N>
inline std::ostream&
operator<<(std::ostream& os, const SkelVector<Scalar,N>& vec)
{
    for (int i=0; i<N-1; i++)
        os << vec[i] << " ";
    os << vec[N-1];
    return os;
}


/// Scalar * vector
template <typename Scalar, int N>
inline SkelVector<Scalar,N> operator*(Scalar s, const SkelVector<Scalar,N>& v )
{
    return v*s;
}


/// compute the dot product of two vectors
template <typename Scalar, int N>
inline Scalar dot(const SkelVector<Scalar,N> v0, const SkelVector<Scalar,N>& v1)
{
    Scalar p(0.0);
    for (int i=0; i<N; i++)
        p += v0[i]*v1[i];
    return p;
}



/// compute the cross product of two vectors (only valid for 3D vectors)
template <typename Scalar, int N>
inline SkelVector<Scalar,3> cross(const SkelVector<Scalar,N> v0, const SkelVector<Scalar,N>& v1)
{
    assert(N>2);
    return SkelVector<Scalar,3>(v0[1]*v1[2]-v0[2]*v1[1],
                            v0[2]*v1[0]-v0[0]*v1[2],
                            v0[0]*v1[1]-v0[1]*v1[0]);
}



//== TYPEDEFS =================================================================


/** 2-byte signed vector */
typedef SkelVector<signed char,2> Vec2c;
/** 2-byte unsigned vector */
typedef SkelVector<unsigned char,2> Vec2uc;
/** 2-short signed vector */
typedef SkelVector<signed short int,2> Vec2s;
/** 2-short unsigned vector */
typedef SkelVector<unsigned short int,2> Vec2us;
/** 2-int signed vector */
typedef SkelVector<signed int,2> Vec2i;
/** 2-int unsigned vector */
typedef SkelVector<unsigned int,2> Vec2ui;
/** 2-float vector */
typedef SkelVector<float,2> Vec2f;
/** 2-double vector */
typedef SkelVector<double,2> Vec2d;

/** 3-byte signed vector */
typedef SkelVector<signed char,3> Vec3c;
/** 3-byte unsigned vector */
typedef SkelVector<unsigned char,3> Vec3uc;
/** 3-short signed vector */
typedef SkelVector<signed short int,3> Vec3s;
/** 3-short unsigned vector */
typedef SkelVector<unsigned short int,3> Vec3us;
/** 3-int signed vector */
typedef SkelVector<signed int,3> Vec3i;
/** 3-int unsigned vector */
typedef SkelVector<unsigned int,3> Vec3ui;
/** 3-float vector */
typedef SkelVector<float,3> Vec3f;
/** 3-double vector */
typedef SkelVector<double,3> Vec3d;

/** 4-byte signed vector */
typedef SkelVector<signed char,4> Vec4c;
/** 4-byte unsigned vector */
typedef SkelVector<unsigned char,4> Vec4uc;
/** 4-short signed vector */
typedef SkelVector<signed short int,4> Vec4s;
/** 4-short unsigned vector */
typedef SkelVector<unsigned short int,4> Vec4us;
/** 4-int signed vector */
typedef SkelVector<signed int,4> Vec4i;
/** 4-int unsigned vector */
typedef SkelVector<unsigned int,4> Vec4ui;
/** 4-float vector */
typedef SkelVector<float,4> Vec4f;
/** 4-double vector */
typedef SkelVector<double,4> Vec4d;


/// @}

#ifdef QT_CORE_LIB
template <typename Scalar, int N>
QDebug operator<<(QDebug dbg, const SkelVector<Scalar,N> &vec){
    dbg.nospace() << "Vector(";
    for (int i=0; i<N-1; i++)
        dbg << vec[i] << " ";
    dbg << vec[N-1];
    dbg << ")";
    return dbg.space();
}
#endif

} // namespace
