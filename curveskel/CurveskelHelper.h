#pragma once
#include "CurveskelQForEach.h"

#include <string>
using namespace std;

namespace CurveskelTypes{

	/// @{ Default property names definition
	const string VPOINT = "v:point";
	const string ELENGTH = "e:length";
	/// @}

	typedef MyWingedMesh::Vertex_property<Point>  Vector3VertexProperty;
	typedef MyWingedMesh::Edge_property<Scalar>	ScalarEdgeProperty;


	class CurveskelHelper{
	protected:
		CurveskelModel* skel;
		Vector3VertexProperty points;
		ScalarEdgeProperty elenght;  /// NULL

	public:
		CurveskelHelper(CurveskelModel* skel) : skel(skel){
			points = skel->vertex_property<Point>("v:point");
			elenght = skel->get_edge_property<Scalar>(ELENGTH);
		}

		ScalarEdgeProperty computeEdgeLengths(string property=ELENGTH){
			elenght = skel->edge_property<Scalar>(property,0.0f);
			foreach(Edge eit,skel->edges())
				elenght[eit] = skel->edge_length(eit);
			return elenght;
		}
	};

}
