#include "voromat.h"
#include "helpers/StatisticsHelper.h"
#include "helpers/ColorizeHelper.h"
#include <QElapsedTimer>
#include "StarlabDrawArea.h"

#ifdef MATLAB
#include "MatlabVoronoiHelper.h"
#endif

#ifdef QHULL
#include "VoronoiHelper.h"
#endif

void filter::applyFilter(RichParameterSet* pars){
    /// Draw the input vertices if overlay was required
    if(pars->getBool(overlayInput)){
        Vector3VertexProperty points = mesh()->get_vertex_property<Vector3>(VPOINT);
        foreach(Vertex v, mesh()->vertices())
            drawArea()->drawPoint(points[v],1,Qt::red);
    }
    QElapsedTimer timer;
            
#ifdef MATLAB
    /// Compute voronoi mapping and measures
    MatlabVoronoiHelper mat(mesh, drawArea);
    timer.start(); // Don't count matlab startup in timing
    mat.createVertexIndexes();
    mat.meshVerticesToVariable("points");
    mat.meshNormalsToVariable("normals");
    mat.computeVoronoiDiagramOf("points");
    mat.searchVoronoiPoles("poleof","scorr");
    mat.getMedialSpokeAngleAndRadii("Vangle","Vradii");
    
    /// Export angle/radii from medial to surface
    mat.eval("vangle=Vangle(poleof);");
    mat.eval("vradii=Vradii(poleof);");   
    mat.variableToVertexScalarProperty("vangle",VANGLE);
    mat.variableToVertexScalarProperty("vradii",VRADII);
    mat.eval("points = loci(poleof,:);");
    
    /// Should we apply the transform?
    string propname = pars->getBool(embedVertices)?VPOINT:VPOLE;
    mat.variableToVector3VertexProperty("points",propname);
#endif

#if 1
    bool isEmbed = pars->getBool(embedVertices);

    timer.start();

    VoronoiHelper h(mesh(), drawArea());
    h.computeVoronoiDiagram();
    h.searchVoronoiPoles();
    h.getMedialSpokeAngleAndRadii();
    h.setToMedial(isEmbed);
#endif

    qDebug() << "[VOROMAT]" << timer.elapsed() << "ms";
            
    /// Colorize one of the exposed properties
    if( pars->getBool(colorizeRadii) || pars->getBool(colorizeAngle) ){
        drawArea()->setRenderer(mesh(),"Smooth");
        string propname;
        if( pars->getBool(colorizeRadii) ) propname = VRADII;
        if( pars->getBool(colorizeAngle) ) propname = VANGLE;
        ColorizeHelper(mesh(),unsignedColorMap).vscalar_to_vcolor(propname);
        // qDebug() << ScalarStatisticsHelper(mesh).statistics(propname);
    }
    
#if 1
    Vector3VertexProperty points = mesh()->get_vertex_property<Vector3>(VPOINT);
    ScalarVertexProperty  vradii = mesh()->get_vertex_property<Scalar>(VRADII);
    SurfaceMeshModel* model = new SurfaceMeshModel("", "cloud");
    document()->addModel(model);
    foreach(Vertex v, mesh()->vertices()){
        if(vradii[v]<3.14*.9)
            model->add_vertex( points[v] );
    }
#endif
}

Q_EXPORT_PLUGIN(voromat)
