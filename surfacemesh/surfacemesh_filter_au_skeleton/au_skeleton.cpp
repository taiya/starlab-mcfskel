#include <QElapsedTimer>

#include "au_skeleton.h"
#include "StarlabDrawArea.h"
#include "SurfaceMeshTypes.h"
#include "SkeletonModel.h"
#include "SkeletonHelper.h"

#include "src/SkeletonExtract.h"

void filter_au_skeleton::initParameters(Document* document, RichParameterSet* parameters, StarlabDrawArea* drawArea)
{
    SurfaceMeshModel* mesh = qobject_cast<SurfaceMeshModel*>(document->selectedModel());

    Scalar edgelength_TH = .01 * mesh->getBoundingBox().size().length();

	parameters->addParam(new RichFloat("laplacianWeight", 1.0,"Laplacian Constraint Weight", ""));
	parameters->addParam(new RichFloat("laplacianScale", 2.0,"Laplacian Constraint Scale", ""));
	parameters->addParam(new RichFloat("posWeight", 1.0,"Positional Constraint Weight", ""));
	parameters->addParam(new RichFloat("posScale", 1.5,"Positional Constraint Scale", ""));
	parameters->addParam(new RichFloat("origPosWeight", 0.0,"Original Positional Constraint Weight", ""));
	parameters->addParam(new RichFloat("shapeEnergyWeight", 0.5,"Shape Energy Weight", ""));
	parameters->addParam(new RichFloat("postSimplifyErrRatio", 0.1,"Post Simplify Error Ratio", ""));
	parameters->addParam(new  RichBool("isApplyJointMerging", false,"Apply Joint Merging", ""));
}

void filter_au_skeleton::applyFilter(Document* document, RichParameterSet* pars, StarlabDrawArea* drawArea){
    SurfaceMeshModel* mesh = qobject_cast<SurfaceMeshModel*>(document->selectedModel());

    /// Set to transparent
    mesh->renderer()->setRenderMode("Transparent");

    if(!mesh) throw StarlabException("FAIL");

	// Get parameters
	Scalar laplacianWeight		= pars->getFloat("laplacianWeight");
	Scalar laplacianScale		= pars->getFloat("laplacianScale");
	Scalar posWeight			= pars->getFloat("posWeight");
	Scalar posScale				= pars->getFloat("posScale");
	Scalar origPosWeight		= pars->getFloat("origPosWeight");
	Scalar shapeEnergyWeight	= pars->getFloat("shapeEnergyWeight");
	Scalar postSimplifyErrRatio	= pars->getFloat("postSimplifyErrRatio");
	bool isApplyJointMerging	= pars->getBool("isApplyJointMerging");

	QSurfaceMesh s (mesh);

    qDebug() << "Started skeleton extraction process..";
    QElapsedTimer timer; timer.start();

	SkeletonExtract extractor (&s, laplacianWeight, laplacianScale, posWeight, 
		posScale, origPosWeight, shapeEnergyWeight, postSimplifyErrRatio, isApplyJointMerging);

    qDebug() << "done ( " << timer.elapsed() << " ms)" ;


#if 0
	Surface_mesh::Vertex_property<Point> newPoints = s.get_vertex_property<Point>("v:point");
    Surface_mesh::Vertex_property<Point> points = mesh->get_vertex_property<Point>("v:point");
	for (Surface_mesh::Vertex_iterator vit = mesh->vertices_begin(); vit != mesh->vertices_end(); ++vit){
		points[vit] = newPoints[vit];
	}
#endif

    /// Create a new "skeletal" model and add it to document
    SkeletonModel* skelModel = new SkeletonModel("","skeleton");
    document->addModel(skelModel);

    Skeleton * skeleton = new Skeleton;
    extractor.SaveToSkeleton(skeleton);

    // Add skeleton vertices
    foreach(SkeletonNode n, skeleton->nodes)
        skelModel->add_vertex(SkeletonTypes::Vector3(n.x(), n.y(), n.z()));

    // Add skeleton edges
    foreach(SkeletonEdge e, skeleton->edges)
        skelModel->add_edge(SkeletonTypes::Vertex(e.n1->index),
                            SkeletonTypes::Vertex(e.n2->index));

	/// And update it after, so we can reset the viewpoint
	mesh->updateBoundingBox();
}

Q_EXPORT_PLUGIN(filter_au_skeleton)
