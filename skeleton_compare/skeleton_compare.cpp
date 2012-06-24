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
    std::vector<double> difference;

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

    difference.resize(skeletons.size() - 1, 0.0);

    SkeletonModel * src = skeletons.front();
    std::vector<Point> src_points;
    SkeletonModel::Vertex_property<SkeletonTypes::Point> src_pnts = src->vertex_property<SkeletonTypes::Point>("v:point");

    foreach(SkeletonTypes::Vertex v, src->vertices())
        src_points.push_back( toKDPoint(src_pnts[v]) );

    // KD Tree, and other used variables
    KDTree src_tree(src_points);
    int closeidx = -1;
    double di = 0.0;
    int skel_idx = 0;

    foreach(SkeletonModel * target, skeletons)
    {
        if(src == target) continue;

        double currDifference = 0.0;

        SkeletonModel::Vertex_property<SkeletonTypes::Point> target_pnts = target->vertex_property<SkeletonTypes::Point>("v:point");

        foreach(SkeletonTypes::Vertex v, target->vertices())
        {
            // Locate closest point, return distance
            src_tree.closest_point( toKDPoint( target_pnts[v] ), closeidx, di);
            currDifference += di;
        }

        difference[skel_idx++] = currDifference;
    }

    // Print stats:
    qDebug() << "Skeletons in list (" << skeletons.size() << ")";
    qDebug() << "Comparing against " << src->name;

    for(int i = 0; i < difference.size(); i++)
        qDebug() << "Difference with ( " << skeletons[i+1]->name << " ) = " << difference[i];
}

Q_EXPORT_PLUGIN(skeleton_compare)
