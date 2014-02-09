#include "skeleton_compare.h"

#include <QMessageBox>
#include "CurveskelModel.h"
#include "CurveskelHelper.h"
#include "KDTree.h"
#include "StarlabDrawArea.h"

std::vector<double> toKDPoint(const CurveskelTypes::Point & from){
    std::vector<double> p(3, 0.0);
    p[0] = from.x(); p[1] = from.y(); p[2] = from.z();
    return p;
}

void skeleton_compare::initParameters(RichParameterSet* pars){
    /// QMap of skeleton models indexed by name
    skeletons.clear();
    foreach(Starlab::Model* model, document()->models()){
        if(model==document()->selectedModel()) continue;
        CurveskelModel* skel = qobject_cast<CurveskelModel*>(model);
        if(skel) skeletons.insert(model->name, skel);
    }
    if(skeletons.size()<1) 
        throw StarlabException("Comparison requires two pre-loaded skeleton models");
    
    pars->addParam( new RichStringSet("Target skeleton", skeletons.keys(), "Target skeleton","The skeleton model to compare *toward*") );
    pars->addParam( new RichBool("Show measurements", false, "Show measurements", "Show blue edge connecting corresponding points") );
}

void skeleton_compare::applyFilter(RichParameterSet* params){
    drawArea()->deleteAllRenderObjects();
    
    //bool show_measurements = params->getBool("Show measurements");
    
    CurveskelModel* src = NULL;
    QString src_name = params->getString("Target Skeleton");
    foreach(Starlab::Model* curr, document()->models())
        if(curr->name == src_name)
            src = qobject_cast<CurveskelModel*>( curr );
    Q_ASSERT(src);
    
    CurveskelModel* target = qobject_cast<CurveskelModel*>( model() );
    if(!target) throw StarlabException("Must be a skeleton model");
    
    CurveskelModel::Vertex_property<CurveskelTypes::Point> src_pnts = src->vertex_property<CurveskelTypes::Point>("v:point");
    CurveskelModel::Vertex_property<CurveskelTypes::Point> target_pnts = target->vertex_property<CurveskelTypes::Point>("v:point");
    
    // Construct a kd-tree
    std::vector<KDPoint> src_points;
    foreach(CurveskelTypes::Vertex v, src->vertices())
        src_points.push_back( toKDPoint(src_pnts[v]) );
    KDTree src_tree(src_points);

    // Common variables
    int closeidx = -1;
    double di = 0.0;

    // Compare target points
    double avgDifference=0.0;
    {
        foreach(CurveskelTypes::Vertex v, target->vertices()){
            // Locate closest point, return distance
            src_tree.closest_point( toKDPoint( target_pnts[v] ), closeidx, di);
            avgDifference += di;
            
            //if(show_measurements)
            //    drawArea()->drawSegment( target_pnts[v], src_pnts[ CurveskelTypes::Vertex(closeidx) ], 2, Qt::blue);
        }
        avgDifference /= target->n_vertices();
        double bbox_diag = src->bbox().diagonal().norm();
        avgDifference /= bbox_diag;
    }

    /// @todo use dialog of sorts..
    qDebug() <<"One sided distance (bbox-norm)" << target->name << "=> " << src->name << avgDifference; 
}
