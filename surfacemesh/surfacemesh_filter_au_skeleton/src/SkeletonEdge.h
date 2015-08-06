#pragma once

#include "SkeletonNode.h"

class SkeletonEdge
{
public:

	uint index;

	SkeletonNode * n1;
	SkeletonNode * n2;

	double length;

	SkeletonEdge(SkeletonNode * N1, SkeletonNode * N2, uint newIndex){
		n1 = N1;
		n2 = N2;

		index = newIndex;
	}

	inline double calculateLength()
	{	return length = n1->distanceTo(*n2);	}

	inline Vec3d direction() const
	{	return *n2 - *n1;	}

	// Functions to help walk on edges
	Vec3d pointAt( double time ) const
	{
		double dist = time * length;
		Vec3d dir = direction().normalized();
		return *n1 + (dir * dist);
	}

	double timeAt( const Vec3d& point )
	{
		return (point - *n1).norm() / length;
	}

	std::pair<double,double> lengthsAt( const Vec3d& point )
	{
		double dist1 = (point - *n1).norm();
		double dist2 = length - dist1;

		return std::pair<double,double>(dist1, dist2);
	}

	std::pair<double,double> lengthsAt( double time )
	{
		// bounded
		if(time > 1.0) time = 1.0;
		if(time < 0.0) time = 0.0;

		return std::pair<double,double>(length * time, length * (1.0 - time));
	}
};
