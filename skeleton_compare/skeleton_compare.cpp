#include "skeleton_compare.h"
#include "SkeletonModel.h"
#include "SkeletonHelper.h"
#include "KDTree.h"

std::vector<double> toKDPoint(const SkeletonTypes::Point & from){
    std::vector<double> p(3, 0.0);
    p[0] = from.x(); p[1] = from.y(); p[2] = from.z();
    return p;
}

void skeleton_compare::initParameters(Document *, RichParameterSet* /*pars*/, StarlabDrawArea *){}

void skeleton_compare::applyFilter(Document* document, RichParameterSet* params, StarlabDrawArea* drawArea)
{
    QList<SkeletonModel*> skeletons;

    // Extract list of all skeleton models
    foreach(Model * m, document->models)
    {
        SkeletonModel* currSkel = qobject_cast<SkeletonModel*>(m);
        if(!currSkel) continue;
        skeletons.push_back(currSkel);
    }

    if(skeletons.size() < 2){
        qDebug() << "Warning: nothing to compare.";
        return;
    }

    SkeletonModel * src = skeletons.front();
    std::vector<Point> src_points;
    SkeletonModel::Vertex_property<SkeletonTypes::Point> src_pnts = src->vertex_property<SkeletonTypes::Point>("v:point");
    foreach(SkeletonTypes::Vertex v, src->vertices())
        src_points.push_back( toKDPoint(src_pnts[v]) );

    // KD Tree
    KDTree src_tree(src_points);

    // Average differences
    std::vector<double> avg_difference (skeletons.size() - 1, 0.0);

    // Common variables
    int closeidx = -1, skel_idx = 0;;
    double di = 0.0;

    // For each skeleton loaded
    foreach(SkeletonModel * target, skeletons)
    {
        if(src == target) continue;

        double sumDifference = 0.0;

        // Compare target points
        SkeletonModel::Vertex_property<SkeletonTypes::Point> target_pnts = target->vertex_property<SkeletonTypes::Point>("v:point");
        foreach(SkeletonTypes::Vertex v, target->vertices())
        {
            // Locate closest point, return distance
            src_tree.closest_point( toKDPoint( target_pnts[v] ), closeidx, di);
            sumDifference += di;
        }

        // Average the sum
        avg_difference[skel_idx++] = sumDifference / target->n_vertices();
    }

    // TODO:
    double bbox_diag = 1.0;

    // Print stats:
    qDebug() << "Skeletons in list (" << skeletons.size() << ")";
    qDebug() << "Comparing against " << src->name;

    for(int i = 0; i < (int)avg_difference.size(); i++)
    {
        double d = avg_difference[i] / bbox_diag;
        qDebug() << "Difference with ( " << skeletons[i+1]->name << " ) = " << d;
    }
}

Q_EXPORT_PLUGIN(skeleton_compare)
