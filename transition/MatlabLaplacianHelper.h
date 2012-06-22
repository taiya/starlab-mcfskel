#pragma once
#include "SurfaceMeshHelper.h"
#include "LegacyLaplacianHelper.h"
#include "MatlabSurfaceMeshHelper.h"

class MatlabMeanValueLaplacianHelper : public MeanValueLaplacianHelper, public MatlabSurfaceMeshHelper{
public:
    MatlabMeanValueLaplacianHelper(SurfaceMeshModel* mesh) : 
        SurfaceMeshHelper(mesh), 
        LaplacianHelper(mesh), 
        MeanValueLaplacianHelper(mesh), 
        MatlabSurfaceMeshHelper(mesh){}
};
