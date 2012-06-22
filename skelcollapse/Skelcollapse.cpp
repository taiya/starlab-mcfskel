#include "Skelcollapse.h"
#include "surfacemesh_io_helpers.h"

//#include "aux_voronoi_poles.cpp"
#include "aux_medial_contraction.cpp"

void Skelcollapse::applyFilter(Document* document, RichParameterSet* pars, StarlabDrawArea* drawArea){
    /// Filter initialization
    this->drawArea = drawArea;
    this->mesh = qobject_cast<SurfaceMeshModel*>( document->selectedModel() );
    drawArea->deleteAllRenderObjects();
        
    /// Run Algorthm
    algorithm(pars);
            
    /// Update internal state
    toggle = !toggle;
    firststep = false;
    stepcount++;
    
    return;
}   

Q_EXPORT_PLUGIN(Skelcollapse)
