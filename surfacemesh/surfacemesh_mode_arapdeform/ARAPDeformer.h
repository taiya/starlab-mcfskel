// Adapted from code by Shaoting Zhang
// http://sourceforge.net/projects/meshtools/
#pragma once

#include "SurfaceMeshHelper.h"

#include <Eigen/Core>
#include <Eigen/Sparse>
#include <Eigen/CholmodSupport>
#include <Eigen/SVD>
#include <Eigen/Geometry>
using namespace Eigen;

class ARAPDeformer{

public:
	ARAPDeformer(Surface_mesh * usingMesh);
	~ARAPDeformer();

private:
	void ComputeCotWeights();
	void BuildAndFactor();
	void SVDRotation();

public:
	void Deform(int ARAPIteration = 1);

private:
	std::vector<Matrix3d> R;
	std::vector<Vector3d> OrigMesh;
	std::vector<VectorXd> xyz;
	std::vector<VectorXd> b;

	// Frequently used
	int nVerts;
    Surface_mesh::Vertex_property<Surface_mesh::Point> points;
    Surface_mesh::Vertex_property<Surface_mesh::Normal> normals;
	Surface_mesh::Vertex_iterator vit, vend;
	Surface_mesh::Vertex_property< std::map<Surface_mesh::Vertex, double> > wij_weight;
	Surface_mesh::Vertex_property< bool > isAnchorPoint, isControlPoint;
	Surface_mesh::Vertex_around_vertex_circulator vvit, vvend;

	SparseMatrix<double> At;
    CholmodSupernodalLLT< SparseMatrix<double> > solver;
	bool isSolverReady;

public:
	Surface_mesh * mesh;

	// Control points
    void SetAnchor( const Surface_mesh::Vertex & v ){
        isAnchorPoint[v] = true;
    }

	void UpdateControl( const Surface_mesh::Vertex & v, const Eigen::Vector3d & newPos ){
		isControlPoint[v] = true;
		points[v] = newPos;
		isSolverReady = false;
	}

	void ClearAnchors(){
		for (vit = mesh->vertices_begin(); vit != vend; ++vit)
			isAnchorPoint[vit] = false;
		isSolverReady = false;
	}

	void ClearControl(){
		for (vit = mesh->vertices_begin(); vit != vend; ++vit)
			isControlPoint[vit] = false;
		isSolverReady = false;
	}

	void ClearAll(){
		ClearAnchors();
		ClearControl();
	}

    Surface_mesh::Halfedge find_halfedge(Surface_mesh::Vertex start, Surface_mesh::Vertex end){
        Surface_mesh::Halfedge h  = mesh->halfedge(start);
        const Surface_mesh::Halfedge hh = h;
        if (h.is_valid()){
            do{
                if (mesh->to_vertex(h) == end)return h;
                h = mesh->cw_rotated_halfedge(h);
            } while (h != hh);
        }
        return Surface_mesh::Halfedge();
    }
};
