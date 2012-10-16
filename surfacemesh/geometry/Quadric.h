//=============================================================================
// Copyright (C) 2001-2005 by Computer Graphics Group, RWTH Aachen
// Copyright (C) 2011 by Graphics & Geometry Group, Bielefeld University
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public License
// as published by the Free Software Foundation, version 2.
//
// This library is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================


#ifndef QUADRIC_H
#define QUADRIC_H


//== INCLUDES =================================================================

#include <surface_mesh/Surface_mesh.h>


//== CLASS DEFINITION =========================================================


/// \addtogroup geometry geometry
/// @{


/** This class stores a quadric as a symmetrix 4x4 matrix. Used by the
    error quadric mesh decimation algorithms.
*/
class Quadric
{
public:


  /// construct quadric from upper triangle of symmetrix 4x4 matrix
  Quadric(double _a, double _b, double _c, double _d,
	                   double _e, double _f, double _g,
                                double _h, double _i,
	                                         double _j)
    : a(_a), b(_b), c(_c), d(_d),
             e(_e), f(_f), g(_g),
                    h(_h), i(_i),
                           j(_j)
  {}


  /// constructor quadric from given plane equation: ax+by+cz+d=0
  Quadric(double _a=0.0, double _b=0.0, double _c=0.0, double _d=0.0)
    :  a(_a*_a), b(_a*_b),  c(_a*_c),  d(_a*_d),
                 e(_b*_b),  f(_b*_c),  g(_b*_d),
                            h(_c*_c),  i(_c*_d),
                                       j(_d*_d)
  {}


  // construct from point and normal specifying a plane
  Quadric(const Normal& n, const Point& p)
  {
    *this = Quadric(n[0], n[1], n[2], -dot(n,p));
  }


  /// set all matric entries to zero
  void clear()  { a = b = c = d = e = f = g = h = i = j = 0.0; }


  /// add given quadric to this quadric
  Quadric& operator+=( const Quadric& _q )
  {
    a += _q.a;  b += _q.b;  c += _q.c;  d += _q.d;
                e += _q.e;  f += _q.f;  g += _q.g;
                            h += _q.h;  i += _q.i;
			                            j += _q.j;
    return *this;
  }


  /// multiply quadric by a scalar
  Quadric& operator*=( double _s)
  {
    a *= _s;  b *= _s;  c *= _s;  d *= _s;
              e *= _s;  f *= _s;  g *= _s;
                        h *= _s;  i *= _s;
                                  j *= _s;
    return *this;
  }


  // evaluate quadric Q at position p by computing (p^T * Q * p)
  double operator()(const Point& p) const
  {
    const double x(p[0]), y(p[1]), z(p[2]);
    return a*x*x + 2.0*b*x*y + 2.0*c*x*z + 2.0*d*x
                 +     e*y*y + 2.0*f*y*z + 2.0*g*y
	                     +     h*z*z + 2.0*i*z
                                         +     j;
  }



private:

  double a, b, c, d,
            e, f, g,
               h, i,
                  j;
};


/// @}


//=============================================================================
#endif
//=============================================================================
