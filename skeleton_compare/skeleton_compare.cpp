#include "skeleton_compare.h"

#include <QMessageBox>
#include "SkeletonModel.h"
#include "SkeletonHelper.h"
#include "KDTree.h"
#include "StarlabDrawArea.h"

std::vector<double> toKDPoint(const SkeletonTypes::Point & from){
    std::vector<double> p(3, 0.0);
    p[0] = from.x(); p[1] = from.y(); p[2] = from.z();
    return p;
}

void skeleton_compare::applyFilter(Document* document, RichParameterSet* params, StarlabDrawArea* drawArea){
    drawArea->deleteAllRenderObjects();
    
    bool show_measurements = params->getBool("Show measurements");
    
    SkeletonModel* src = document->getModel<SkeletonModel>(params->getString("Target Skeleton"));
    SkeletonModel* target = qobject_cast<SkeletonModel*>( document->selectedModel() );
    if(!target) throw StarlabException("Must be a skeleton model");
    
    SkeletonModel::Vertex_property<SkeletonTypes::Point> src_pnts = src->vertex_property<SkeletonTypes::Point>("v:point");
    SkeletonModel::Vertex_property<SkeletonTypes::Point> target_pnts = target->vertex_property<SkeletonTypes::Point>("v:point");
    
    // Construct a kd-tree
    std::vector<Point> src_points;
    foreach(SkeletonTypes::Vertex v, src->vertices())
        src_points.push_back( toKDPoint(src_pnts[v]) );
    KDTree src_tree(src_points);

    // Common variables
    int closeidx = -1;
    double di = 0.0;

    // Compare target points
    double avgDifference=0.0;
    {
        foreach(SkeletonTypes::Vertex v, target->vertices()){
            // Locate closest point, return distance
            src_tree.closest_point( toKDPoint( target_pnts[v] ), closeidx, di);
            avgDifference += di;
            
            if(show_measurements)
                drawArea->drawSegment( target_pnts[v], src_pnts[ SkeletonTypes::Vertex(closeidx) ], 2, Qt::blue);            
        }
        avgDifference /= target->n_vertices();
        double bbox_diag = src->getBoundingBox().size().length();
        avgDifference /= bbox_diag;
    }

    /// @todo use dialog of sorts..
    qDebug() <<"One sided distance (bbox-norm)" << target->name << "=> " << src->name << avgDifference; 
}

Q_EXPORT_PLUGIN(skeleton_compare)
