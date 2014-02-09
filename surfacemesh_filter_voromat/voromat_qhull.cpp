#include "voromat.h"
#include "helpers/StatisticsHelper.h"
#include "helpers/ColorizeHelper.h"
#include <QElapsedTimer>
#include "StarlabDrawArea.h"
#include "QhullVoronoiHelper.h"

void filter::applyFilter(RichParameterSet* pars){
    /// Draw the input vertices if overlay was required
    if(pars->getBool(overlayInput)){
        Vector3VertexProperty points = mesh()->get_vertex_property<Vector3>(VPOINT);
        foreach(Vertex v, mesh()->vertices())
            drawArea()->drawPoint(points[v],1,Qt::red);
    }
            
    bool isEmbed = pars->getBool(embedVertices);

    // We need normals
    mesh()->update_face_normals();
    mesh()->update_vertex_normals();

    QElapsedTimer timer;
    timer.start();
        VoronoiHelper h(mesh(), drawArea());
        h.computeVoronoiDiagram();
        h.searchVoronoiPoles();
        h.getMedialSpokeAngleAndRadii();
        h.setToMedial(isEmbed);
    qDebug() << "[VOROMAT]" << timer.elapsed() << "ms";
            
    /// Colorize one of the exposed properties
    if( pars->getBool(colorizeRadii) || pars->getBool(colorizeAngle) ){
        drawArea()->setRenderer(mesh(),"Smooth");
        std::string propname;
        if( pars->getBool(colorizeRadii) ) propname = VRADII;
        if( pars->getBool(colorizeAngle) ) propname = VANGLE;
        ColorizeHelper(mesh(),unsignedColorMap).vscalar_to_vcolor(propname);
        // qDebug() << ScalarStatisticsHelper(mesh).statistics(propname);
    }

#if 0
    /// Test of lambda medial axis
	drawArea()->deleteAllRenderObjects();
	Vector3VertexProperty vpoles = mesh()->get_vertex_property<Vector3>(VPOLE);
	ScalarVertexProperty  vangles = mesh()->get_vertex_property<Scalar>(VANGLE);
	SurfaceMeshModel* model = new SurfaceMeshModel("", "cloud");
	qDebug() << StatisticsHelper<Scalar>(mesh()).statistics(VANGLE);
	document()->addModel(model);
	drawArea()->setRenderer(mesh(),"Transparent");
	drawArea()->setRenderer(model, "Vertices as Dots");
	foreach(Vertex v, mesh()->vertices()){
	    if(vangles[v] > 3.1415*.4)
	        drawArea()->drawPoint(vpoles[v]);
	}
#endif
}
