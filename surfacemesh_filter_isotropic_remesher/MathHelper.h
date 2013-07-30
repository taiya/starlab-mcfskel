#pragma once

#include "SurfaceMeshModel.h"
#include <limits>

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

    const Vector3& n0 = normal[mesh.face(_heh)];
    const Vector3& n1 = normal[mesh.face(mesh.opposite_halfedge(_heh))];

    Vector3 he = points[mesh.to_vertex(_heh)] - points[mesh.from_vertex(_heh)];

	Scalar da_cos = dot(n0, n1);

	//should be normalized, but we need only the sign
    Scalar da_sin_sign = dot(n0.cross(n1), he);
	return angle(da_cos, da_sin_sign);
}

static Scalar distPointTriangleSquared( const Vector3& _p,const Vector3& _v0,const Vector3& _v1,const Vector3& _v2,Vector3& _nearestPoint )
{
	Vector3 v0v1 = _v1 - _v0;
	Vector3 v0v2 = _v2 - _v0;
    Vector3 n = v0v1.cross(v0v2); // not normalized !
    double d = n.squaredNorm();


	// Check if the triangle is degenerated
    if (d < std::numeric_limits<float>::min() && d > -std::numeric_limits<float>::min()) {
		std::cerr << "distPointTriangleSquared: Degenerated triangle !\n";
        return std::numeric_limits<float>::max();
	}
	double invD = 1.0 / d;


	// these are not needed for every point, should still perform
	// better with many points against one triangle
	Vector3 v1v2 = _v2 - _v1;
    double inv_v0v2_2 = 1.0 / v0v2.squaredNorm();
    double inv_v0v1_2 = 1.0 / v0v1.squaredNorm();
    double inv_v1v2_2 = 1.0 / v1v2.squaredNorm();


	Vector3 v0p = _p - _v0;
    Vector3 t = v0p.cross(n);
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
            s12 = dot( v1v2, Vector3(_p - _v1 )) * inv_v1v2_2;
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
            s12 = dot( v1v2, Vector3(_p - _v1 )) * inv_v1v2_2;
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
        s12 = dot( v1v2, Vector3(_p - _v1 )) * inv_v1v2_2;
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
        return (_nearestPoint - _p).squaredNorm();
	}

	_nearestPoint = v0p;

    return (_nearestPoint - _p).squaredNorm();
}

inline double ClosestPointTriangle(Vector3 p, Vector3 a, Vector3 b, Vector3 c, Vector3 & closest)
{
    // Check if P in vertex region outside A
    Vec3d ab = b - a;
    Vec3d ac = c - a;
    Vec3d ap = p - a;
    double d1 = dot(ab, ap);
    double d2 = dot(ac, ap);
    if (d1 <= 0 && d2 <= 0)
    {
        closest = a;
        return (p-closest).squaredNorm(); // barycentric coordinates (1,0,0)
    }
    // Check if P in vertex region outside B
    Vec3d bp = p - b;
    double d3 = dot(ab, bp);
    double d4 = dot(ac, bp);
    if (d3 >= 0 && d4 <= d3)
    {
        closest = b;
        return (p-closest).squaredNorm(); // barycentric coordinates (0,1,0)
    }
    // Check if P in edge region of AB, if so return projection of P onto AB
    double vc = d1*d4 - d3*d2;
    if (vc <= 0 && d1 >= 0 && d3 <= 0) {
        double v = d1 / (d1 - d3);
        closest = a + v * ab;
        return (p - closest).squaredNorm(); // barycentric coordinates (1-v,v,0)
    }
    // Check if P in vertex region outside C
    Vec3d cp = p - c;
    double d5 = dot(ab, cp);
    double d6 = dot(ac, cp);
    if (d6 >= 0 && d5 <= d6)
    {
        closest = c;
        return (p-closest).squaredNorm(); // barycentric coordinates (0,0,1)
    }
    // Check if P in edge region of AC, if so return projection of P onto AC
    double vb = d5*d2 - d1*d6;
    if (vb <= 0 && d2 >= 0 && d6 <= 0) {
        double w = d2 / (d2 - d6);
        closest = a + w * ac;
        return (p - closest).squaredNorm(); // barycentric coordinates (1-w,0,w)
    }
    // Check if P in edge region of BC, if so return projection of P onto BC
    double va = d3*d6 - d5*d4;
    if (va <= 0 && (d4 - d3) >= 0 && (d5 - d6) >= 0) {
        double w = (d4 - d3) / ((d4 - d3) + (d5 - d6));
        closest = b + w * (c - b);
        return (p - closest).squaredNorm(); // barycentric coordinates (0,1-w,w)
    }
    // P inside face region. Compute Q through its barycentric coordinates (u,v,w)
    double denom = 1.0 / (va + vb + vc);
    double v = vb * denom;
    double w = vc * denom;
    closest = a + ab * v + ac * w;
    return (p - closest).squaredNorm(); // = u*a + v*b + w*c, u = va * denom = 1 - v - w
}

inline static bool TestSphereTriangle(Point sphereCenter, double sphereRadius, Point a, Point b, Point c, Point &p)
{
    // Find point P on triangle ABC closest to sphere center
    ClosestPointTriangle(sphereCenter, a, b, c, p);

    // Sphere and triangle intersect if the (squared) distance from sphere
    // center to point p is less than the (squared) sphere radius
    Vec3d v = p - sphereCenter;
    return dot(v, v) <= sphereRadius * sphereRadius;
}

inline static bool TestSphereTriangle(Point sphereCenter, double sphereRadius, Point a, Point b, Point c){
    Vector3 p(0,0,0);
    return TestSphereTriangle(sphereCenter, sphereRadius, a, b, c);
}
