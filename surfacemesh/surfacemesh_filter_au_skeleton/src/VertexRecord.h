#pragma once

#include <float.h>

// data structures
#include <queue>
#include <QList>
#include <QMap>
#include <QSet>
#include <QVector>

#include <set>

#include "SurfaceMeshModel.h"
#include "SurfaceMeshTypes.h"
using namespace SurfaceMeshTypes;

// Eigen is used for sparse matrix (and solving)
#include <Eigen/Core>
#include <Eigen/Sparse>
#include <Eigen/Geometry>
using namespace Eigen;


// Subset of QSurfaceMesh from 'ennetws@gmail.com'
class QSurfaceMesh: public SurfaceMeshModel{
public:

	QSurfaceMesh(SurfaceMeshModel * from){
		Surface_mesh::operator=(*from);
	}

	Point bbmin, bbmax, center;
	Scalar radius;
	double scalingFactor;

	void computeBoundingBox(){
		Vertex_property<Point> points = vertex_property<Point>("v:point");
		Vertex_iterator vit, vend = vertices_end();

		// compute bounding box
		this->bbmin = Vec3d( DBL_MAX-1, DBL_MAX-1, DBL_MAX-1);
		this->bbmax = -bbmin;

		center = Vec3d(0,0,0);

		for (vit = vertices_begin(); vit != vend; ++vit)
		{
			bbmin.minimize(points[vit]);
			bbmax.maximize(points[vit]);
		}

		center = (bbmin + bbmax) * 0.5f;
		radius = (bbmax - bbmin).norm() * 0.5f;
	}

	void moveCenterToOrigin(){
		computeBoundingBox();

		Surface_mesh::Vertex_property<Point> points = vertex_property<Point>("v:point");
		Surface_mesh::Vertex_iterator vit, vend = vertices_end();

		for (vit = vertices_begin(); vit != vend; ++vit){
			if (!is_deleted(vit))
				points[vit] -= center;
		}

		computeBoundingBox();
	}

	std::vector<Vec3d> facePoints( Face f ){
		Vertex_property<Point>  points  = vertex_property<Point>("v:point");
		Vertex_around_face_circulator fvit, fvend;

		fvit = fvend = vertices(f);

		std::vector<Vec3d> v;

		do{ v.push_back(points[fvit]); } while (++fvit != fvend);

		return v;
	}

	double faceArea( Face f ){
		std::vector<Vec3d> v = facePoints(f);
		Vec3d t = cross((v[1] - v[0]), (v[2] - v[0]));
		return 0.5 * t.norm();
	}

	std::vector<Vertex_iterator> vertex_array;
	std::vector<Face_iterator> face_array;

	void assignVertexArray(){
		Vertex_iterator vit, vend = vertices_end();

		for(vit = vertices_begin(); vit != vend; ++vit)
			vertex_array.push_back(vit);
	}

	void assignFaceArray(){
		Face_iterator fit, fend = faces_end();

		for(fit = faces_begin(); fit != fend; ++fit)
			face_array.push_back(fit);
	}

	double volume(){
		double totalVolume = 0;
		Face_iterator fit, fend = faces_end();

		for(fit = faces_begin(); fit != fend; ++fit){
			std::vector<Point> facePnts = this->facePoints(fit);

			Point a = facePnts[0];
			Point b = facePnts[1];
			Point c = facePnts[2];

			totalVolume +=
				a.x() * b.y() * c.z() - a.x() * b.z() * c.y() -
				a.y() * b.x() * c.z() + a.y() * b.z() * c.x() +
				a.z() * b.x() * c.y() - a.z() * b.y() * c.x();
		}
		return totalVolume;
	}

	double normalize(){
		this->computeBoundingBox();

		Point d = bbmax - bbmin;
		double s = (d.x() > d.y())? d.x():d.y();
		s = (s>d.z())? s: d.z();

		Vertex_property<Point>  points  = vertex_property<Point>("v:point");
		Vertex_iterator vit, vend = vertices_end();

		for(vit = vertices_begin(); vit != vend; ++vit)
			points[vit] /= s;

		return scalingFactor = s;
	}

	std::vector<Point> clonePoints()
	{
		std::vector<Point> result;
		Vertex_property<Point>  points  = vertex_property<Point>("v:point");
		Vertex_iterator vit, vend = vertices_end();

		for(vit = vertices_begin(); vit != vend; ++vit)
			result.push_back(points[vit]);

		return result;
	}

	Point getVertexPos( const Vertex v )
	{
		Vertex_property<Point> points = vertex_property<Point>("v:point");
		return points[v];
	}

	Point getVertexPos( uint v_id ) { return getVertexPos(Vertex(v_id)); }

	void setFromPoints( const std::vector<Point>& fromPoints )
	{
		Vertex_property<Point>  points  = vertex_property<Point>("v:point");
		Vertex_iterator vit, vend = vertices_end();

		for(vit = vertices_begin(); vit != vend; ++vit)
			points[vit] = fromPoints[Vertex(vit).idx()];
	}

	std::set<uint> vertexIndicesAroundVertex( const Vertex& v ){
		std::set<uint> result;

		Surface_mesh::Vertex_around_vertex_circulator vit, vend;
		vit = vend = vertices(v);
		do{ 
            Vertex v = vit;
            result.insert(v.idx()); ++vit;
        } while(vit != vend);

		return result;
	}

	std::set<uint> faceIndicesAroundVertex( const Vertex& v ){
		std::set<uint> result;

		Face_around_vertex_circulator fit, fend;
		fit = fend = Surface_mesh::faces(v);
		do{ 
            Face f = fit;
            result.insert(f.idx()); ++fit; 
        } while(fit != fend);

		return result;
	}

	std::vector<unsigned int> triangles, edges;

	void fillTrianglesList()
	{
		// get face indices
		Face_iterator fit, fend = faces_end();
		Vertex_around_face_circulator fvit, fvend;
		Vertex v0, v1, v2;

		triangles.clear();

		for (fit = faces_begin(); fit != fend; ++fit)
		{
			fvit = fvend = vertices(fit);
			v0 = fvit; v1 = ++fvit; v2 = ++fvit;

			triangles.push_back(v0.idx());
			triangles.push_back(v1.idx());
			triangles.push_back(v2.idx());
		}
	}
};


class VertexRecord {

public:
	Vec3d pos;

	std::set<uint> adjV;
	std::set<uint> adjF;
	std::set<uint> collapseFrom;

	std::map<uint, double> boundaryLength;

	int vIndex;
	int minIndex;
	double minError;
	bool center;
	double err;
	double nodeSize;
	int PQIndex;

	MatrixXd matrix;

	VertexRecord()
	{
		// Defaults
		vIndex = minIndex = -1;
		minError = std::numeric_limits<double>::max();
		center = false;
		err = 0;
		nodeSize = 1;
		matrix = Matrix4d::Zero(4,4);
		PQIndex = -1;
	}

	VertexRecord(QSurfaceMesh & mesh, const Surface_mesh::Vertex & v)
	{
		// Defaults
		vIndex = minIndex = -1;
		minError = std::numeric_limits<double>::max();
		center = false;
		err = 0;
		nodeSize = 1;
		matrix = Matrix4d::Zero(4,4);
		PQIndex = -99;

		Surface_mesh::Vertex_property< std::set<uint> > adjVV = mesh.vertex_property< std::set<uint> >("v:adjVV");
		Surface_mesh::Vertex_property< std::set<uint> > adjVF = mesh.vertex_property< std::set<uint> >("v:adjVF");

		vIndex = v.idx();

		// Get vertex position
		Surface_mesh::Vertex_property<Point> points = mesh.get_vertex_property<Point>("v:point");
		pos = points[v];

		adjV = adjVV[v];
		adjF = adjVF[v];
	}

	int CompareTo(const VertexRecord &other) const
	{ 
		if (minError < other.minError) return -1;
		if (minError > other.minError) return 1;
		return 0;
	}
};
