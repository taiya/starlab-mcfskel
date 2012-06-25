#include "skeleton_resample.h"
#include "StarlabDrawArea.h"

void skeleton_resample::initParameters(Document *document, RichParameterSet *parameters, StarlabDrawArea *drawArea){
   parameters->addParam(new RichFloat("Edge length", 0.1));
}

void skeleton_resample::applyFilter(Model* model,RichParameterSet* pars, StarlabDrawArea* drawarea){

    this->skel = qobject_cast<SkeletonModel*>(model);
    this->skel_points = skel->vertex_property<SkeletonTypes::Point>("v:point");

    double threshold = pars->getFloat("Edge length");

    foreach(SkeletonModel::Edge e, skel->edges())
        recursiveSplitEdge(e, threshold);

    /// After splitting them visualize the density
    foreach(SkeletonModel::Vertex v, skel->vertices())
        drawarea->drawPoint(skel_points[v],2,Qt::blue);
    
    //skel->garbage_collection(); // NOT IMPLEMENTED YET!
}

void skeleton_resample::recursiveSplitEdge(SkeletonModel::Edge e, double threshold)
{
    SkeletonModel::Vertex v1 = skel->vertex(e, 0);
    SkeletonModel::Vertex v2 = skel->vertex(e, 1);

    SkeletonTypes::Vector3 p1 = skel_points[v1];
    SkeletonTypes::Vector3 p2 = skel_points[v2];

    // Break condition
    if((p2 - p1).norm() <= threshold)
        return;

   SkeletonTypes::Vector3 midPoint(0.5 * (p1 + p2));

    // 1) Remove edge record from two verts and skeleton
    skel->remove_edge(v1, e);
    skel->remove_edge(v2, e);
    skel->delete_edge(e);

    // 2) Add middle vertex to skeleton
    SkeletonModel::Vertex midVert = skel->add_vertex(midPoint);

    // 3) Add two new edges v1 - midVert and v2 - midVert, recursive calls
    recursiveSplitEdge(skel->add_edge(v1, midVert), threshold);
    recursiveSplitEdge(skel->add_edge(v2, midVert), threshold);
}

Q_EXPORT_PLUGIN(skeleton_resample)
