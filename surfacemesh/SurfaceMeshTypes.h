#pragma once
#include "geometry/Vector.h"
#include <QVector3D>
#include <string> 

namespace SurfaceMeshTypes{
    /// Scalar type
    typedef double             Scalar;
    /// To count stuff
    typedef unsigned int       Counter;
    /// To index stuff (i.e. matlab pointer)
    typedef unsigned int       Index;
    /// To index stuff (i.e. matlab pointer)
    typedef unsigned int       Size;
    /// Vector type
    typedef Vector<Scalar,3> Vector3;
    /// Point type
    typedef Vector3   Point;
    /// Normal type
    typedef Vector3   Normal;
    /// Color type
    typedef Vector3   Color;
    /// Texture coordinate type
    typedef Vector3   Texture_coordinate;
}
