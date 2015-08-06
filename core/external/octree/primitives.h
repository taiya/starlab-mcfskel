#pragma once

#include <float.h>
#include "Eigen/Dense"

namespace{
    Eigen::Vector3d cross(Eigen::Vector3d a, Eigen::Vector3d b){ return a.cross(b); }
    double dot(Eigen::Vector3d a, Eigen::Vector3d b){ return a.dot(b); }
}

/* AABB-triangle overlap test code                      */
/* by Tomas Akenine-Möller                              */
#define X 0
#define Y 1
#define Z 2

#define FINDMINMAX(x0,x1,x2,min,max) \
	min = max = x0;   \
	if(x1<min) min=x1;\
	if(x1>max) max=x1;\
	if(x2<min) min=x2;\
	if(x2>max) max=x2;

/*======================== X-tests ========================*/
#define AXISTEST_X01(a, b, fa, fb)			   \
    p0 = a*v0[Y] - b*v0[Z];			       	   \
    p2 = a*v2[Y] - b*v2[Z];			       	   \
    if(p0<p2) {min=p0; max=p2;} else {min=p2; max=p0;} \
    rad = fa * boxhalfsize[Y] + fb * boxhalfsize[Z];   \
    if(min>rad || max<-rad) return 0;
#define AXISTEST_X2(a, b, fa, fb)			   \
    p0 = a*v0[Y] - b*v0[Z];			           \
    p1 = a*v1[Y] - b*v1[Z];			       	   \
    if(p0<p1) {min=p0; max=p1;} else {min=p1; max=p0;} \
    rad = fa * boxhalfsize[Y] + fb * boxhalfsize[Z];   \
    if(min>rad || max<-rad) return 0;
/*======================== Y-tests ========================*/
#define AXISTEST_Y02(a, b, fa, fb)			   \
    p0 = -a*v0[X] + b*v0[Z];		      	   \
    p2 = -a*v2[X] + b*v2[Z];	       	       	   \
    if(p0<p2) {min=p0; max=p2;} else {min=p2; max=p0;} \
    rad = fa * boxhalfsize[X] + fb * boxhalfsize[Z];   \
    if(min>rad || max<-rad) return 0;
#define AXISTEST_Y1(a, b, fa, fb)			   \
    p0 = -a*v0[X] + b*v0[Z];		      	   \
    p1 = -a*v1[X] + b*v1[Z];	     	       	   \
    if(p0<p1) {min=p0; max=p1;} else {min=p1; max=p0;} \
    rad = fa * boxhalfsize[X] + fb * boxhalfsize[Z];   \
    if(min>rad || max<-rad) return 0;
/*======================== Z-tests ========================*/
#define AXISTEST_Z12(a, b, fa, fb)			   \
    p1 = a*v1[X] - b*v1[Y];			           \
    p2 = a*v2[X] - b*v2[Y];			       	   \
    if(p2<p1) {min=p2; max=p1;} else {min=p1; max=p2;} \
    rad = fa * boxhalfsize[X] + fb * boxhalfsize[Y];   \
    if(min>rad || max<-rad) return 0;
#define AXISTEST_Z0(a, b, fa, fb)			   \
    p0 = a*v0[X] - b*v0[Y];				   \
    p1 = a*v1[X] - b*v1[Y];			           \
    if(p0<p1) {min=p0; max=p1;} else {min=p1; max=p0;} \
    rad = fa * boxhalfsize[X] + fb * boxhalfsize[Y];   \
    if(min>rad || max<-rad) return 0;

static inline int planeBoxOverlap(const Eigen::Vector3d& normal, const Eigen::Vector3d& vert, const Eigen::Vector3d& maxbox)
{
    Eigen::Vector3d vmin,vmax;
	for(int q=X; q<=Z; q++){
		double v = vert[q];					
		if(normal[q] > 0.0){
			vmin[q]=-maxbox[q] - v;
			vmax[q]= maxbox[q] - v;
		}
		else{
			vmin[q]= maxbox[q] - v;
			vmax[q]=-maxbox[q] - v;
		}
	}
    if(normal.dot(vmin) > 0.0) return 0;
    if(normal.dot(vmax) >= 0.0) return 1;

	return 0;
}

#undef X
#undef Y
#undef Z

struct Ray
{
    Eigen::Vector3d origin;
    Eigen::Vector3d direction;
	int index;
	double thickness;

    Ray(const Eigen::Vector3d & Origin = Eigen::Vector3d(), const Eigen::Vector3d & Direction = Eigen::Vector3d(), double Thickness = 0.0, int Index = -1) : origin(Origin), index(Index){
		direction = Direction.normalized();
		thickness = Thickness;
	}

	inline Ray inverse() const { return Ray(origin, -direction); } 

	Ray& operator= (const Ray& other){
		this->origin = other.origin;
		this->direction = other.direction;
		this->index = other.index;
		this->thickness = other.thickness;

		return *this;
	}
};

class BoundingBox
{

public:
    Eigen::Vector3d center;
    Eigen::Vector3d vmax, vmin;
	double xExtent, yExtent, zExtent;

	BoundingBox()
	{
        this->center = Eigen::Vector3d(0,0,0);

		this->xExtent = 0;
		this->yExtent = 0;
		this->zExtent = 0;
	}

    BoundingBox( const Eigen::Vector3d& c, double x, double y, double z )
	{
		this->center = c;

		this->xExtent = x;
		this->yExtent = y;
		this->zExtent = z;

        Eigen::Vector3d corner(x, y, z);

		vmin = center - corner;
		vmax = center + corner;
	}

	BoundingBox& operator=( const BoundingBox& other )
	{
		this->center = other.center;

		this->xExtent = other.xExtent;
		this->yExtent = other.yExtent;
		this->zExtent = other.zExtent;

		this->vmax = other.vmax;
		this->vmin = other.vmin;

		return *this;
	}

    void computeFromTris( const std::vector< std::vector<Eigen::Vector3d> > & tris )
	{
        vmin = Eigen::Vector3d(DBL_MAX, DBL_MAX, DBL_MAX);
		vmax = -vmin;

		double minx = 0, miny = 0, minz = 0;
		double maxx = 0, maxy = 0, maxz = 0;

		minx = maxx = tris[0][0].x();
		miny = maxy = tris[0][1].y();
		minz = maxz = tris[0][2].z();

		for (int i = 0; i < (int)tris.size(); i++)
		{
			for(int v = 0; v < 3; v++)
			{
                Eigen::Vector3d vec = tris[i][v];

				if (vec.x() < minx) minx = vec.x();
				if (vec.x() > maxx) maxx = vec.x();
				if (vec.y() < miny) miny = vec.y();
				if (vec.y() > maxy) maxy = vec.y();
				if (vec.z() < minz) minz = vec.z();
				if (vec.z() > maxz) maxz = vec.z();
			}
		}

        vmax = Eigen::Vector3d(maxx, maxy, maxz);
        vmin = Eigen::Vector3d(minx, miny, minz);

		this->center = (vmin + vmax) / 2.0;

		this->xExtent = fabs(vmax.x() - center.x());
		this->yExtent = fabs(vmax.y() - center.y());
		this->zExtent = fabs(vmax.z() - center.z());
	}

    std::vector<Eigen::Vector3d> getCorners()
	{
        std::vector<Eigen::Vector3d> corners;

        Eigen::Vector3d x = (Eigen::Vector3d(1,0,0) * xExtent);
        Eigen::Vector3d y = (Eigen::Vector3d(0,1,0) * yExtent);
        Eigen::Vector3d z = (Eigen::Vector3d(0,0,1) * zExtent);

        Eigen::Vector3d c = center + x + y + z;

		corners.push_back(c);
		corners.push_back(c - (x*2));
		corners.push_back(c - (y*2));
		corners.push_back(c - (x*2) - (y*2));

		corners.push_back(corners[0] - (z*2));
		corners.push_back(corners[1] - (z*2));
		corners.push_back(corners[2] - (z*2));
		corners.push_back(corners[3] - (z*2));

		return corners;
	}

	bool intersects( const Ray& ray ) const
	{
		// r.dir is unit direction vector of ray
        Eigen::Vector3d dirfrac;
		dirfrac.x() = 1.0f / ray.direction.x();
		dirfrac.y() = 1.0f / ray.direction.y();
		dirfrac.z() = 1.0f / ray.direction.z();

		double overlap = ray.thickness;
        Eigen::Vector3d minv = vmin + ((vmin - center).normalized() * overlap);
        Eigen::Vector3d maxv = vmax + ((vmax - center).normalized() * overlap);

		// lb is the corner of AABB with minimal coordinates - left bottom, rt is maximal corner
		// ray.origin is origin of ray
		float t1 = (minv.x() - ray.origin.x())*dirfrac.x();
		float t2 = (maxv.x() - ray.origin.x())*dirfrac.x();
		float t3 = (minv.y() - ray.origin.y())*dirfrac.y();
		float t4 = (maxv.y() - ray.origin.y())*dirfrac.y();
		float t5 = (minv.z() - ray.origin.z())*dirfrac.z();
		float t6 = (maxv.z() - ray.origin.z())*dirfrac.z();

		float tmin = qMax(qMax(qMin(t1, t2), qMin(t3, t4)), qMin(t5, t6));
		float tmax = qMin(qMin(qMax(t1, t2), qMax(t3, t4)), qMax(t5, t6));

		double t = 0;

		// if tmax < 0, ray (line) is intersecting AABB, but whole AABB is behing us
		if (tmax < 0)
		{
			t = tmax;
			return false;
		}

		// if tmin > tmax, ray doesn't intersect AABB
		if (tmin > tmax)
		{
			t = tmax;
			return false;
		}

		t = tmin;
		return true;
	}

	bool intersectsWorking( const Ray& ray )
	{
        Eigen::Vector3d T_1, T_2; // vectors to hold the T-values for every direction
		double t_near = -DBL_MAX; // maximums defined in float.h
		double t_far = DBL_MAX;

		//vmin = center - Vec3d(xExtent);
		//vmax = center + Vec3d(xExtent);

		for (int i = 0; i < 3; i++){ //we test slabs in every direction
			if (ray.direction[i] == 0){ // ray parallel to planes in this direction
				if ((ray.origin[i] < vmin[i]) || (ray.origin[i] > vmax[i])) {
					return false; // parallel AND outside box : no intersection possible
				}
			} else { // ray not parallel to planes in this direction
				T_1[i] = (vmin[i] - ray.origin[i]) / ray.direction[i];
				T_2[i] = (vmax[i] - ray.origin[i]) / ray.direction[i];

				if(T_1[i] > T_2[i]){ // we want T_1 to hold values for intersection with near plane
					std::swap(T_1,T_2);
				}
				if (T_1[i] > t_near){
					t_near = T_1[i];
				}
				if (T_2[i] < t_far){
					t_far = T_2[i];
				}
				if( (t_near > t_far) || (t_far < 0) ){
					return false;
				}
			}
		}
		double tnear = t_near, tfar = t_far; // put return values in place
		Q_UNUSED(tnear);
		Q_UNUSED(tfar);
		return true; // if we made it here, there was an intersection - YAY
	}

	bool intersectsOld( const Ray& ray ) const
	{
		double rhs;
		double fWdU[3];
		double fAWdU[3];
		double fDdU[3];
		double fADdU[3];
		double fAWxDdU[3];

        Eigen::Vector3d UNIT_X(1.0, 0.0, 0.0);
        Eigen::Vector3d UNIT_Y(0.0, 1.0, 0.0);
        Eigen::Vector3d UNIT_Z(0.0, 0.0, 1.0);

        Eigen::Vector3d diff = ray.origin - center;
        Eigen::Vector3d wCrossD = cross(ray.direction , diff);

		fWdU[0] = dot(ray.direction , UNIT_X);
		fAWdU[0] = fabs(fWdU[0]);
		fDdU[0] = dot(diff , UNIT_X);
		fADdU[0] = fabs(fDdU[0]);
		if (fADdU[0] > xExtent && fDdU[0] * fWdU[0] >= 0.0)		return false;

		fWdU[1] = dot(ray.direction , UNIT_Y);
		fAWdU[1] = fabs(fWdU[1]);
		fDdU[1] = dot(diff , UNIT_Y);
		fADdU[1] = fabs(fDdU[1]);
		if (fADdU[1] > yExtent && fDdU[1] * fWdU[1] >= 0.0)		return false;

		fWdU[2] = dot(ray.direction , UNIT_Z);
		fAWdU[2] = fabs(fWdU[2]);
		fDdU[2] = dot(diff , UNIT_Z);
		fADdU[2] = fabs(fDdU[2]);
		if (fADdU[2] > zExtent && fDdU[2] * fWdU[2] >= 0.0)		return false;

		fAWxDdU[0] = fabs(dot(wCrossD , UNIT_X));
		rhs = yExtent * fAWdU[2] + zExtent * fAWdU[1];
		if (fAWxDdU[0] > rhs)		return false;

		fAWxDdU[1] = fabs(dot(wCrossD , UNIT_Y));
		rhs = xExtent * fAWdU[2] + zExtent * fAWdU[0];
		if (fAWxDdU[1] > rhs)		return false;

		fAWxDdU[2] = fabs(dot(wCrossD , UNIT_Z));
		rhs = xExtent * fAWdU[1] + yExtent * fAWdU[0];
		if (fAWxDdU[2] > rhs)		return false;

		return true;
	}

	/* AABB-triangle overlap test code                      */
	/* by Tomas Akenine-Möller                              */
    bool containsTriangle( const Eigen::Vector3d& tv0, const Eigen::Vector3d& tv1, const Eigen::Vector3d& tv2 ) const
	{
        Eigen::Vector3d boxcenter(center);
        Eigen::Vector3d boxhalfsize(xExtent, yExtent, zExtent);

		int X = 0, Y = 1, Z = 2;

		/*    use separating axis theorem to test overlap between triangle and box */
		/*    need to test for overlap in these directions: */
		/*    1) the {x,y,z}-directions (actually, since we use the AABB of the triangle */
		/*       we do not even need to test these) */
		/*    2) normal of the triangle */
		/*    3) crossproduct(edge from tri, {x,y,z}-directin) */
		/*       this gives 3x3=9 more tests */
        Eigen::Vector3d v0,v1,v2;
		double min,max,p0,p1,p2,rad,fex,fey,fez;
        Eigen::Vector3d normal,e0,e1,e2;

		/* This is the fastest branch on Sun */
		/* move everything so that the box center is in (0,0,0) */
		v0=tv0-boxcenter;
		v1=tv1-boxcenter;
		v2=tv2-boxcenter;
		
		/* compute triangle edges */
		e0=v1-v0;      /* tri edge 0 */
		e1=v2-v1;      /* tri edge 1 */
		e2=v0-v2;      /* tri edge 2 */
		
		/* Bullet 3:  */
		/*  test the 9 tests first (this was faster) */
		fex = fabsf(e0[X]);
		fey = fabsf(e0[Y]);
		fez = fabsf(e0[Z]);
		AXISTEST_X01(e0[Z], e0[Y], fez, fey);
		AXISTEST_Y02(e0[Z], e0[X], fez, fex);
		AXISTEST_Z12(e0[Y], e0[X], fey, fex);
		fex = fabsf(e1[X]);
		fey = fabsf(e1[Y]);
		fez = fabsf(e1[Z]);
		AXISTEST_X01(e1[Z], e1[Y], fez, fey);
		AXISTEST_Y02(e1[Z], e1[X], fez, fex);
		AXISTEST_Z0(e1[Y], e1[X], fey, fex);
		fex = fabsf(e2[X]);
		fey = fabsf(e2[Y]);
		fez = fabsf(e2[Z]);
		AXISTEST_X2(e2[Z], e2[Y], fez, fey);
		AXISTEST_Y1(e2[Z], e2[X], fez, fex);
		AXISTEST_Z12(e2[Y], e2[X], fey, fex);
		
		/* Bullet 1: */
		/*  first test overlap in the {x,y,z}-directions */
		/*  find min, max of the triangle each direction, and test for overlap in */
		/*  that direction -- this is equivalent to testing a minimal AABB around */
		/*  the triangle against the AABB */
		/* test in X-direction */
		FINDMINMAX(v0[X],v1[X],v2[X],min,max);
		if(min>boxhalfsize[X] || max<-boxhalfsize[X]) return 0;
		/* test in Y-direction */
		FINDMINMAX(v0[Y],v1[Y],v2[Y],min,max);
		if(min>boxhalfsize[Y] || max<-boxhalfsize[Y]) return 0;
		/* test in Z-direction */
		FINDMINMAX(v0[Z],v1[Z],v2[Z],min,max);
		if(min>boxhalfsize[Z] || max<-boxhalfsize[Z]) return 0;
		
		/* Bullet 2: */
		/*  test if the box intersects the plane of the triangle */
		/*  compute plane equation of triangle: normal*x+d=0 */
		normal = cross(e0, e1);

		if(!planeBoxOverlap(normal,v0,boxhalfsize)) return 0;
		return 1;   /* box and triangle overlaps */
	}

	bool intersectsBoundingBox( const BoundingBox& bb ) const
	{
		if (center.x() + xExtent < bb.center.x() - bb.xExtent || center.x() - xExtent > bb.center.x() + bb.xExtent)
			return false;
		else if (center.y() + yExtent < bb.center.y() - bb.yExtent || center.y() - yExtent > bb.center.y() + bb.yExtent)
			return false;
		else if (center.z() + zExtent < bb.center.z() - bb.zExtent || center.z() - zExtent > bb.center.z() + bb.zExtent)
			return false;
		else
			return true;
	}

    bool intersectsSphere( const Eigen::Vector3d& sphere_center, double radius )
	{
		if (fabs(center.x() - sphere_center.x()) < radius + xExtent
			&& fabs(center.y() - sphere_center.y()) < radius + yExtent
			&& fabs(center.z() - sphere_center.z()) < radius + zExtent)
			return true;

		return false;
	}

    bool contains( const Eigen::Vector3d& point ) const
	{
		return fabs(center.x() - point.x()) < xExtent
			&& fabs(center.y() - point.y()) < yExtent
			&& fabs(center.z() - point.z()) < zExtent;
	}

    Eigen::Vector3d Center()
	{
		return center;
	}

	double Diag()
	{
		return (vmin - vmax).norm();
	}
};

struct HitResult{
	bool hit;
	double distance;

	double u;
	double v;
	int index;

	HitResult(bool isHit = false, double hitDistance = DBL_MAX) : hit(isHit), distance(hitDistance)
	{
		u = -1.0;
		v = -1.0;
		index = -1;
	}

	HitResult(const HitResult& other)
	{
		this->hit = other.hit;
		this->distance = other.distance;
		this->u = other.u;
		this->v = other.v;
		this->index = other.index;
	}

	HitResult& operator= (const HitResult& other)
	{
		this->hit = other.hit;
		this->distance = other.distance;
		this->u = other.u;
		this->v = other.v;
		this->index = other.index;

		return *this;
	}
};
