#pragma once
#include <string>
#include "WingedgeMesh.h"
#include "./Vector.h"

namespace CurveskelTypes{
/// @{ Basic types
    typedef SkelVector<double,3> Vector3;
	typedef Vector3 Point;
    typedef double Scalar;
    typedef std::string String;
/// @}

/// @{ Default property names
    const String VCOORD = "v:point";
/// @}

/// @{ WidgedgeMesh types
    typedef WingedgeMesh<Scalar,Vector3> MyWingedMesh;
    typedef MyWingedMesh::Vertex Vertex;
    typedef MyWingedMesh::Edge   Edge;
	typedef MyWingedMesh::Edge_around_vertex		Edge_around_vertex;
    typedef MyWingedMesh::Vertex_property<Vector3>	Vector3VertexProperty;
	typedef MyWingedMesh::Face_property<Vector3>	Vector3FaceProperty;
	typedef MyWingedMesh::Vertex_property<Scalar>	ScalarVertexProperty;
	typedef MyWingedMesh::Edge_property<Scalar>		ScalarEdgeProperty;
	typedef MyWingedMesh::Vertex_property<bool>		BoolVertexProperty;
	typedef MyWingedMesh::Edge_property<bool>		BoolEdgeProperty;
	typedef MyWingedMesh::Vertex_property<int>		IntVertexProperty;
	typedef MyWingedMesh::Edge_property<int>		IntEdgeProperty;
/// @}
}
