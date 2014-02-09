#include "skeleton_resample.h"
#include "StarlabDrawArea.h"

void skeleton_resample::initParameters(RichParameterSet *parameters){
   parameters->addParam(new RichFloat("Edge length", 0.001f));
}

void skeleton_resample::applyFilter(RichParameterSet* pars){
    drawArea()->deleteAllRenderObjects();
    
    this->skel = qobject_cast<CurveskelModel*>(model());
    this->skel_points = skel->vertex_property<CurveskelTypes::Point>("v:point");

    double threshold = pars->getFloat("Edge length");

    foreach(CurveskelModel::Edge e, skel->edges())
        recursiveSplitEdge(e, threshold);

    /// After splitting them visualize the density
    //foreach(CurveskelModel::Vertex v, skel->vertices())
    //    drawArea()->drawPoint(skel_points[v],2,Qt::blue);
    
    /// Print out average edge length
    double avg=0;
    foreach(CurveskelModel::Edge e, skel->edges())
        avg += skel->edge_length(e);
    avg /= skel->n_edges();

    double bbox_diag = skel->bbox().diagonal().norm();
    double bboxnorm_avg = avg/bbox_diag;
    
    qDebug() << skel->name << "resampled to average edge length:" << avg << ", w.r.t. bbox: " << bboxnorm_avg;
    skel->garbage_collection();
}

void skeleton_resample::recursiveSplitEdge(CurveskelModel::Edge e, double threshold)
{
    CurveskelModel::Vertex v1 = skel->vertex(e, 0);
    CurveskelModel::Vertex v2 = skel->vertex(e, 1);

    CurveskelTypes::Vector3 p1 = skel_points[v1];
    CurveskelTypes::Vector3 p2 = skel_points[v2];

    // Break condition
    if((p2 - p1).norm() <= threshold)
        return;

   CurveskelTypes::Vector3 midPoint(0.5 * (p1 + p2));

    // 1) Remove edge record from two verts and skeleton
    skel->remove_edge(v1, e);
    skel->remove_edge(v2, e);
    skel->delete_edge(e);

    // 2) Add middle vertex to skeleton
    CurveskelModel::Vertex midVert = skel->add_vertex(midPoint);

    // 3) Add two new edges v1 - midVert and v2 - midVert, recursive calls
    recursiveSplitEdge(skel->add_edge(v1, midVert), threshold);
    recursiveSplitEdge(skel->add_edge(v2, midVert), threshold);
}
