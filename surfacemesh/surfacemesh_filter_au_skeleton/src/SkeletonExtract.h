// Based on the paper "Skeleton Extraction by Mesh Contraction" [Au:2008]
#pragma once

// For simplification step
#include "VertexRecord.h"
#include "PriorityQueue.h"

// Skeleton data structure
#include "Skeleton.h"

class SkeletonExtract{

public:
	SkeletonExtract( QSurfaceMesh * fromMesh, double laplacianWeight = 1.0, 
		double laplacianScale = 2.0, double posWeight = 1.0, double posScale = 1.5, 
		double origPosWeight = 0.0, double shapeEnergyWeight = 0.5, 
		double postSimplifyErrRatio = 0.1, bool isApplyJointMerging = false);

	// Resulting collapsed vertex positions
	std::vector<Point> collapsedVertexPos, simplifiedVertexPos;
	std::vector<Point> embededVertexPos;
	std::vector<Point> debugPoints, debugPoints2;

	void SaveToSkeleton(Skeleton * s);

private:
	// Contraction Parameters
	double LaplacianConstraintWeight;
	double PositionalConstraintWeight;
	double OriginalPositionalConstraintWeight;

	double LaplacianConstraintScale;
	double PositionalConstraintScale;

	double volumneRatioThreashold;
	double faceAreaThreshold;

	// Simplification Parameters
	double PostSimplifyErrorRatio;
	int remainingVertexCount;
	int TargetVertexCount;

	// Pointer to mesh
    QSurfaceMesh * mesh;
    QSurfaceMesh * src_mesh;

	double scaleFactor;
	double originalArea;
	double originalVolume;

	// Number of vertices, faces
	uint n;
	uint fn;

	// per-vertex
	std::vector<double> lapWeight;
	std::vector<double> posWeight;
	std::vector<double> collapsedLength;

	std::vector<int> vertexFlag;

	// Original structure
	std::vector<double> originalFaceArea;
	std::vector<Vec3d> originalVertexPos;

	// Steps
	void GeometryCollapse(int maxIter = 10);
	void Simplification();
	void EmbeddingImproving();

	// Geometry collapse sub-steps:
	Eigen::SparseMatrix<double> A, ATA;
	Eigen::SparseMatrix<double> BuildMatrixA();
	void ImplicitSmooth();

	// Simplification sub-steps:
	void SetupLocalAdjacenciesLists();
	void UpdateVertexRecords(VertexRecord&);
	QVector<VertexRecord> simplifiedVertexRec;
	QVector<VertexRecord> vRec, vRec_original;
	double ShapeEnergyWeight;
	bool isApplyJointMergingStep;

	// Embedding sub-steps:
	void MergeJoint();
};

#define VEC_TO_EIGEN(v)  ( Eigen::Vector3d(v.x(), v.y(), v.z()) ) 
#define VEC_TO_EIGEN4(v)  ( Eigen::Vector4d(v[0], v[1], v[2], v[3]) ) 

#define EIGEN_TO_VEC(v)  ( Vec3d(v[0], v[1], v[2]) ) 
#define EIGEN4_TO_VEC(v)  ( Vec4d(v(0), v(1), v(2), v(3)) ) 

#define V(vi) ( mesh->vertex_array[vi] )

