#include "CurveskelModel.h"
#include "CurveskelQForEach.h"
#include "StarlabException.h"

using namespace std;
using namespace CurveskelTypes;

CurveskelModel::CurveskelModel(QString path, QString name) : Model(path,name){}

void CurveskelModel::updateBoundingBox(){
    Vector3VertexProperty vcoord = get_vertex_property<Vector3>(VCOORD);
    _bbox.setNull();
    foreach(Vertex v, this->vertices())
        _bbox = _bbox.merged( Eigen::AlignedBox3d( Eigen::Vector3d(vcoord[v]) ) );
}

CurveskelForEachEdgeHelper CurveskelModel::edges(){
    return CurveskelForEachEdgeHelper(this);
}

std::set<Vertex> CurveskelModel::junctions()
{
    std::set<Vertex> j;

    foreach(Vertex v, this->vertices())
        if(this->valence(v) > 2)
            j.insert(v);

    return j;
}

CurveskelForEachVertexHelper CurveskelModel::vertices(){
    return CurveskelForEachVertexHelper(this);
}

std::set<Vertex> CurveskelTypes::CurveskelModel::adjacent_set( Vertex v )
{
	std::set<Vertex> adj;
	Edge_around_vertex e(this, v), eend = e;
	while(!e.end())
	{
		(v == this->vertex(e, 0)) ? adj.insert(this->vertex(e, 1)) : adj.insert(this->vertex(e, 0));
		++e;
	}
	return adj;
}

Vertex CurveskelTypes::CurveskelModel::other_vertex( Edge e, Vertex v )
{
    return (v == this->vertex(e, 0)) ? this->vertex(e, 1) : this->vertex(e, 0);
}

// Explicit Template Instantiation
template class WingedgeMesh <double, CurveskelTypes::SkelVector<double,3> >;
