#include "surfacemesh_filter_ballpivoting.h"
#include "SurfaceMeshModel.h"
#include "RichParameterSet.h"
#include "ball_pivoting.h"

using namespace SurfaceMesh;

void surfacemesh_filter_ballpivoting::initParameters(RichParameterSet* pars){
    pars->addParam( new RichFloat("radius", -1, "Ball radius", "Use -1 to use sampling (recommended), 0 for auto-guess."));
    pars->addParam( new RichInt("face_count", -1, "Face count", "Stop when this count is reached."));
}

void surfacemesh_filter_ballpivoting::applyFilter(RichParameterSet* pars)
{
    SurfaceMeshModel * newModel = new SurfaceMeshModel("", "ball_pivot");

    // Collect point cloud from selected model
    Surface_mesh::Vertex_property<Vector3> points = mesh()->get_vertex_property<Vector3>(VPOINT);
    foreach(Vertex v, mesh()->vertices())
        newModel->add_vertex(points[v]);


    // Reconstruct mesh
    BallPivoting ball(newModel, pars->getFloat("radius"));
    ball.BuildMesh( pars->getInt("face_count") );

    /// Clean up:
    // Remove isolated vertices
    foreach(Vertex v, newModel->vertices())
        if(newModel->is_isolated(v))
            newModel->remove_vertex(v);
    newModel->garbage_collection();

    // Compute normals and such
    newModel->updateBoundingBox();
    newModel->update_face_normals();
    newModel->update_vertex_normals();

    document()->pushBusy();
    document()->addModel( newModel );
    document()->popBusy();
}

Q_EXPORT_PLUGIN(surfacemesh_filter_ballpivoting)
