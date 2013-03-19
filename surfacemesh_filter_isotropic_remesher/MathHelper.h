#pragma once

#include "SurfaceMeshModel.h"

template <class T>
static inline T deg_to_rad(const T& _angle)
{ return M_PI*(_angle/180); }

template <class T>
static inline T rad_to_deg(const T& _angle)
{ return 180*(_angle/M_PI); }

static inline Scalar sane_aarg(Scalar _aarg){
	if (_aarg < -1)
		_aarg = -1;
	else if (_aarg >  1)
		_aarg = 1;
	return _aarg;
}

static inline Scalar angle(Scalar _cos_angle, Scalar _sin_angle)
{//sanity checks - otherwise acos will return NAN
	_cos_angle = sane_aarg(_cos_angle);
	return (Scalar) _sin_angle >= 0 ? acos(_cos_angle) : -acos(_cos_angle);
}

// Helper function
static Scalar calc_dihedral_angle(SurfaceMeshModel & mesh, SurfaceMeshModel::Halfedge _heh)
{
	if (mesh.is_boundary(mesh.edge(_heh)))
	{//the dihedral angle at a boundary edge is 0
		return 0;
	}

	Vector3FaceProperty normal = mesh.face_property<Vector3>( FNORMAL );
	Vector3VertexProperty points = mesh.vertex_property<Vector3>( VPOINT );

	const Normal& n0 = normal[mesh.face(_heh)];
	const Normal& n1 = normal[mesh.face(mesh.opposite_halfedge(_heh))];

	Normal he = points[mesh.to_vertex(_heh)] - points[mesh.from_vertex(_heh)];

	Scalar da_cos = dot(n0, n1);

	//should be normalized, but we need only the sign
	Scalar da_sin_sign = dot(cross(n0, n1), he);
	return angle(da_cos, da_sin_sign);
}

static Scalar distPointTriangleSquared( const Vector3& _p,const Vector3& _v0,const Vector3& _v1,const Vector3& _v2,Vector3& _nearestPoint )
{
	Vector3 v0v1 = _v1 - _v0;
	Vector3 v0v2 = _v2 - _v0;
	Vector3 n = cross(v0v1, v0v2); // not normalized !
	double d = n.sqrnorm();


	// Check if the triangle is degenerated
	if (d < FLT_MIN && d > -FLT_MIN) {
		std::cerr << "distPointTriangleSquared: Degenerated triangle !\n";
		return -1.0;
	}
	double invD = 1.0 / d;


	// these are not needed for every point, should still perform
	// better with many points against one triangle
	Vector3 v1v2 = _v2 - _v1;
	double inv_v0v2_2 = 1.0 / v0v2.sqrnorm();
	double inv_v0v1_2 = 1.0 / v0v1.sqrnorm();
	double inv_v1v2_2 = 1.0 / v1v2.sqrnorm();


	Vector3 v0p = _p - _v0;
	Vector3 t = cross(v0p, n);
	double  s01, s02, s12;
	double a = dot(t, v0v2) * -invD;
	double b = dot(t, v0v1) * invD;


	if (a < 0)
	{
		// Calculate the distance to an edge or a corner vertex
		s02 = dot( v0v2, v0p ) * inv_v0v2_2;
		if (s02 < 0.0)
		{
			s01 = dot( v0v1, v0p ) * inv_v0v1_2;
			if (s01 <= 0.0) {
				v0p = _v0;
			} else if (s01 >= 1.0) {
				v0p = _v1;
			} else {
				v0p = _v0 + v0v1 * s01;
			}
		} else if (s02 > 1.0) {
			s12 = dot( v1v2, ( _p - _v1 )) * inv_v1v2_2;
			if (s12 >= 1.0) {
				v0p = _v2;
			} else if (s12 <= 0.0) {
				v0p = _v1;
			} else {
				v0p = _v1 + v1v2 * s12;
			}
		} else {
			v0p = _v0 + v0v2 * s02;
		}
	} else if (b < 0.0) {
		// Calculate the distance to an edge or a corner vertex
		s01 = dot( v0v1, v0p ) * inv_v0v1_2;
		if (s01 < 0.0)
		{
			s02 = dot( v0v2,  v0p ) * inv_v0v2_2;
			if (s02 <= 0.0) {
				v0p = _v0;
			} else if (s02 >= 1.0) {
				v0p = _v2;
			} else {
				v0p = _v0 + v0v2 * s02;
			}
		} else if (s01 > 1.0) {
			s12 = dot( v1v2, ( _p - _v1 )) * inv_v1v2_2;
			if (s12 >= 1.0) {
				v0p = _v2;
			} else if (s12 <= 0.0) {
				v0p = _v1;
			} else {
				v0p = _v1 + v1v2 * s12;
			}
		} else {
			v0p = _v0 + v0v1 * s01;
		}
	} else if (a+b > 1.0) {
		// Calculate the distance to an edge or a corner vertex
		s12 = dot( v1v2, ( _p - _v1 )) * inv_v1v2_2;
		if (s12 >= 1.0) {
			s02 = dot( v0v2, v0p ) * inv_v0v2_2;
			if (s02 <= 0.0) {
				v0p = _v0;
			} else if (s02 >= 1.0) {
				v0p = _v2;
			} else {
				v0p = _v0 + v0v2*s02;
			}
		} else if (s12 <= 0.0) {
			s01 = dot( v0v1,  v0p ) * inv_v0v1_2;
			if (s01 <= 0.0) {
				v0p = _v0;
			} else if (s01 >= 1.0) {
				v0p = _v1;
			} else {
				v0p = _v0 + v0v1 * s01;
			}
		} else {
			v0p = _v1 + v1v2 * s12;
		}
	} else {
		// Calculate the distance to an interior point of the triangle
		_nearestPoint = _p - n*(dot(n,v0p) * invD);
		return (_nearestPoint - _p).sqrnorm();
	}

	_nearestPoint = v0p;

	return (_nearestPoint - _p).sqrnorm();
}

