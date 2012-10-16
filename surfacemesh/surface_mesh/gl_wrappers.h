//=============================================================================
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

#ifndef GL_WRAPPERS_H
#define GL_WRAPPERS_H


//== INCLUDES =================================================================
#if 0
    // Mac OS X
    #ifdef __APPLE__
    #  include <GLUT/glut.h>
    #  include <OpenGL/gl.h>
    #  include <OpenGL/glu.h>

    // Windows
    #elif _WIN32
    #  include <stdlib.h>
    //#  include <GL/glut.h>
    #  include <GL/gl.h>
    #  include <GL/glu.h>

    // Unix
    #else
    #  include <GL/glut.h>
    #  include <GL/gl.h>
    #  include <GL/glu.h>
    #endif
#endif

#include "geometry/Vector.h"


//== NAMESPACE ================================================================

namespace gl {

//=============================================================================


/// \addtogroup glut_viewer glut_viewer
/// @{

    
//-------------------------------------------------------------------- glVertex

/// Wrapper: glVertex for Vec2i
inline void glVertex(const Vec2i& _v)  { glVertex2i(_v[0], _v[1]); }

/// Wrapper: glVertex for Vec2f
inline void glVertex(const Vec2f& _v)  { glVertex2fv(_v.data()); }

/// Wrapper: glVertex for Vec2d
inline void glVertex(const Vec2d& _v)  { glVertex2dv(_v.data()); }

/// Wrapper: glVertex for Vec3f
inline void glVertex(const Vec3f& _v)  { glVertex3fv(_v.data()); }

/// Wrapper: glVertex for Vec3d
inline void glVertex(const Vec3d& _v)  { glVertex3dv(_v.data()); }

/// Wrapper: glVertex for Vec4f
inline void glVertex(const Vec4f& _v)  { glVertex4fv(_v.data()); }

/// Wrapper: glVertex for Vec4d
inline void glVertex(const Vec4d& _v)  { glVertex4dv(_v.data()); }



//------------------------------------------------------------------- glTexCoord

/// Wrapper: glTexCoord for 1D float
inline void glTexCoord(float _t) { glTexCoord1f(_t); }

/// Wrapper: glTexCoord for 1D double
inline void glTexCoord(double _t) { glTexCoord1d(_t); }

/// Wrapper: glTexCoord for Vec2f
inline void glTexCoord(const Vec2f& _t) { glTexCoord2fv(_t.data()); }

/// Wrapper: glTexCoord for Vec2d
inline void glTexCoord(const Vec2d& _t) { glTexCoord2dv(_t.data()); }

/// Wrapper: glTexCoord for Vec3f
inline void glTexCoord(const Vec3f& _t) { glTexCoord3fv(_t.data()); }

/// Wrapper: glTexCoord for Vec3d
inline void glTexCoord(const Vec3d& _t) { glTexCoord3dv(_t.data()); }

/// Wrapper: glTexCoord for Vec4f
inline void glTexCoord(const Vec4f& _t) { glTexCoord4fv(_t.data()); }

/// Wrapper: glTexCoord for Vec4d
inline void glTexCoord(const Vec4d& _t) { glTexCoord4dv(_t.data()); }



//--------------------------------------------------------------------- glNormal

/// Wrapper: glNormal for Vec3f
inline void glNormal(const Vec3f& _n)  { glNormal3fv(_n.data()); }

/// Wrapper: glNormal for Vec3d
inline void glNormal(const Vec3d& _n)  { glNormal3dv(_n.data()); }



//---------------------------------------------------------------------- glColor

/// Wrapper: glColor for Vec3f
inline void glColor(const Vec3f&  _v)  { glColor3fv(_v.data()); }

/// Wrapper: glColor for Vec3uc
inline void glColor(const Vec3uc& _v)  { glColor3ubv(_v.data()); }

/// Wrapper: glColor for Vec4f
inline void glColor(const Vec4f&  _v)  { glColor4fv(_v.data()); }

/// Wrapper: glColor for Vec4d
inline void glColor(const Vec4d&  _v)  { glColor4dv(_v.data()); }

/// Wrapper: glColor for Vec3d
inline void glColor(const Vec3d&  _v)  { glColor4dv(_v.data()); }

/// Wrapper: glColor for Vec4uc
inline void glColor(const Vec4uc&  _v) { glColor4ubv(_v.data()); }



//-------------------------------------------------------------- glVertexPointer

/// Wrapper: glVertexPointer for Vec2f
inline void glVertexPointer(const Vec2f* _p)
{ ::glVertexPointer(2, GL_FLOAT, 0, _p); }

/// Wrapper: glVertexPointer for Vec2d
inline void glVertexPointer(const Vec2d* _p)
{ ::glVertexPointer(2, GL_DOUBLE, 0, _p); }

/// Wrapper: glVertexPointer for Vec3f
inline void glVertexPointer(const Vec3f* _p)
{ ::glVertexPointer(3, GL_FLOAT, 0, _p); }

/// Wrapper: glVertexPointer for Vec3d
inline void glVertexPointer(const Vec3d* _p)
{ ::glVertexPointer(3, GL_DOUBLE, 0, _p); }

/// Wrapper: glVertexPointer for Vec4f
inline void glVertexPointer(const Vec4f* _p)
{ ::glVertexPointer(4, GL_FLOAT, 0, _p); }

/// Wrapper: glVertexPointer for Vec4d
inline void glVertexPointer(const Vec4d* _p)
{ ::glVertexPointer(4, GL_DOUBLE, 0, _p); }

/// original method
inline void glVertexPointer(GLint n, GLenum t, GLsizei s, const GLvoid *p)
{ ::glVertexPointer(n, t, s, p); }



//-------------------------------------------------------------- glNormalPointer

/// Wrapper: glNormalPointer for Vec3f
inline void glNormalPointer(const Vec3f* _p)
{ ::glNormalPointer(GL_FLOAT, 0, _p); }

/// Wrapper: glNormalPointer for Vec3d
inline void glNormalPointer(const Vec3d* _p)
{ ::glNormalPointer(GL_DOUBLE, 0, _p); }

/// original method
inline void glNormalPointer(GLenum t, GLsizei s, const GLvoid *p)
{ ::glNormalPointer(t, s, p); }



//--------------------------------------------------------------- glColorPointer

/// Wrapper: glColorPointer for Vec3uc
inline void glColorPointer(const Vec3uc* _p)
{ ::glColorPointer(3, GL_UNSIGNED_BYTE, 0, _p); }

/// Wrapper: glColorPointer for Vec3f
inline void glColorPointer(const Vec3f* _p)
{ ::glColorPointer(3, GL_FLOAT, 0, _p); }

/// Wrapper: glColorPointer for Vec3f
inline void glColorPointer(const Vec3d* _p)
{ ::glColorPointer(3, GL_DOUBLE, 0, _p); }

/// Wrapper: glColorPointer for Vec4uc
inline void glColorPointer(const Vec4uc* _p)
{ ::glColorPointer(4, GL_UNSIGNED_BYTE, 0, _p); }

/// Wrapper: glColorPointer for Vec4f
inline void glColorPointer(const Vec4f* _p)
{ ::glColorPointer(4, GL_FLOAT, 0, _p); }

/// original method
inline void glColorPointer(GLint n, GLenum t, GLsizei s, const GLvoid *p)
{ ::glColorPointer(n, t, s, p); }



//------------------------------------------------------------ glTexCoordPointer

/// Wrapper: glTexCoordPointer for float
inline void glTexCoordPointer(const float* _p)
{ ::glTexCoordPointer(1, GL_FLOAT, 0, _p); }

/// Wrapper: glTexCoordPointer for Vec2d
inline void glTexCoordPointer(const double* _p)
{ ::glTexCoordPointer(1, GL_DOUBLE, 0, _p); }

/// Wrapper: glTexCoordPointer for Vec2f
inline void glTexCoordPointer(const Vec2f* _p)
{ ::glTexCoordPointer(2, GL_FLOAT, 0, _p); }

/// Wrapper: glTexCoordPointer for Vec2d
inline void glTexCoordPointer(const Vec2d* _p)
{ ::glTexCoordPointer(2, GL_DOUBLE, 0, _p); }

/// Wrapper: glTexCoordPointer for Vec3f
inline void glTexCoordPointer(const Vec3f* _p)
{ ::glTexCoordPointer(3, GL_FLOAT, 0, _p); }

/// Wrapper: glTexCoordPointer for Vec3d
inline void glTexCoordPointer(const Vec3d* _p)
{ ::glTexCoordPointer(3, GL_DOUBLE, 0, _p); }

/// Wrapper: glTexCoordPointer for Vec4f
inline void glTexCoordPointer(const Vec4f* _p)
{ ::glTexCoordPointer(4, GL_FLOAT, 0, _p); }

/// Wrapper: glTexCoordPointer for Vec4d
inline void glTexCoordPointer(const Vec4d* _p)
{ ::glTexCoordPointer(4, GL_DOUBLE, 0, _p); }

/// original method
inline void glTexCoordPointer(GLint n, GLenum t, GLsizei s, const GLvoid *p)
{ ::glTexCoordPointer(n, t, s, p); }


//-----------------------------------------------------------------------------

    
/// @}
    
    
//=============================================================================
} // namespace gl
//=============================================================================
#endif // GL_WRAPPERS_H
//=============================================================================
