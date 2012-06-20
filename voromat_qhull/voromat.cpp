#include "voromat.h"
#include "VoronoiHelper.h"
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
    VoronoiHelper h(mesh, drawArea);
    h.computeVoronoiDiagram();
	h.searchVoronoiPoles();
	h.getMedialSpokeAngleAndRadii();
    h.setToMedial();

    /// Mark mesh as a medial mesh
    mesh->setProperty("MedialMesh",true);
        
    /// Debug visualization
    bool executethis = true;
    if(executethis){
        mesh->renderer()->setRenderMode("Smooth");
        // const char* prop = "v:angle";
        const char* prop = "v:radii";
        qDebug() << ScalarStatisticsHelper(mesh).statistics(prop);
        ColorizeHelper(mesh,unsignedColorMap).vscalar_to_vcolor(prop);
    }
}

Q_EXPORT_PLUGIN(voromat)
