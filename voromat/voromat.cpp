#include "voromat.h"
#include "MatlabVoronoiHelper.h"
#include "StatisticsHelper.h"
#include "ColorizeHelper.h"

void voromat::applyFilter(Document* doc, RichParameterSet* /*pars*/, StarlabDrawArea* drawArea){
    /// Remove any extra stuff I might have added
    drawArea->deleteAllRenderObjects();

    /// Are you applying filter on an appropriate model?
    Model* selectedModel = doc->selectedModel();
    SurfaceMeshModel* input = qobject_cast<SurfaceMeshModel*>(selectedModel);
    if(!input) throw StarlabException("Selection must be a 'SurfaceMeshModel'");
    input->renderer()->setRenderMode("Transparent");

    /// Create a copy of the model
    QString newpath = "";
    QString newname = QString(input->name).append(" (medial)");
    SurfaceMeshModel* mesh = new SurfaceMeshModel(newpath,newname);
    mesh->deep_copy(*input);
    doc->addModel(mesh);
    doc->setSelectedModel(mesh);
    
    /// Model name might contain hint whether that's the case..    
    /// in particular make sure I don't call voronoi on an already 
    /// medially contracted thingie.
    // if(mesh->name)
    
    /// Compute voronoi mapping and measures
    MatlabVoronoiHelper h(mesh, drawArea);
    h.createVertexIndexes();
    h.meshVerticesToVariable("points");
    h.meshNormalsToVariable("normals");
    h.computeVoronoiDiagramOf("points");
    h.searchVoronoiPoles("poleof","scorr");
    h.getMedialSpokeAngleAndRadii("Vangle","Vradii");
    
    /// Export angle/radii from medial to surface
    h.eval("vangle=Vangle(poleof);");
    h.eval("vradii=Vradii(poleof);");   
    h.variableToVertexScalarProperty("vangle","v:angle");
    h.variableToVertexScalarProperty("vradii","v:radii");
    
    /// Set vertex positions to medial, leave surface untouched in debug mode
    h.eval("points = loci(poleof,:);");
    h.variableToMeshVertices("points");
    
    /// Mark mesh as a medial mesh
    mesh->setProperty("MedialMesh",true);
        
    /// Debug visualization
    bool executethis = false;
    if(executethis){
        mesh->renderer()->setRenderMode("Smooth");
        // const char* prop = "v:angle";
        const char* prop = "v:radii";
        qDebug() << ScalarStatisticsHelper(mesh).statistics(prop);
        ColorizeHelper(mesh,unsignedColorMap).vscalar_to_vcolor(prop);
    }
}

Q_EXPORT_PLUGIN(voromat)
