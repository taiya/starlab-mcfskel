#pragma once
#pragma GCC diagnostic ignored "-Wwrite-strings"

#include <QDir>
#include "interfaces/SurfaceMeshModelPlugins.h"
#include "StarlabDrawArea.h"
#include "ColorizeHelper.h"
#include "CurvatureEstimationHelper.h"
#include "LaplacianHelper.h"
#include "StatisticsHelper.h"
#include "DoUndoHelper.h"
#include "LocalAnalysisHelper.h"
#include "SurfaceAreaHelper.h"

class mcfskel : public SurfaceMeshFilterPlugin{
    Q_OBJECT
    Q_INTERFACES(FilterPlugin)  

public:
    /// Used to store surface correspondences, note you will not be able to use it to 
    /// access the evolving mesh.. these Vertex indexes are w.r.t the ORIGINAL mesh!!
    typedef QList<Surface_mesh::Vertex> VertexList;
    typedef Surface_mesh::Vertex_property<VertexList> VertexListVertexProperty;
    
public:
    QString name() { return "MCF Skeletonization"; }
    QString description() { return "Performs Skeletonization by Collapsing Mesh Structure"; }
    QKeySequence shortcut(){ return QKeySequence(Qt::CTRL + Qt::Key_K); }
    
public:    
    bool toggle; /// TRUE 
    bool firststep;
    Counter stepcount;
    mcfskel() : toggle(true), firststep(true), stepcount(0){}

public:
    void initParameters(SurfaceMeshModel *mesh, RichParameterSet *parameters, StarlabDrawArea */*drawArea*/){
        Scalar scale = 0.002*mesh->getBoundingBox().size().length();
        parameters->addParam(new RichFloat("omega_L_0",1));
        parameters->addParam(new RichFloat("omega_H_0",20));
        parameters->addParam(new RichFloat("omega_P_0",40));
        parameters->addParam(new RichFloat("edgelength_TH",scale));
        parameters->addParam(new RichFloat("zero_TH",1e-10));
        parameters->addParam(new RichBool("performContraction",true));
        parameters->addParam(new RichBool("medialContraction",false));
        /// Harder to do...
        // parameters->addParam(new RichBool("overlayInitial",false));
    }
    
    void applyFilter(SurfaceMeshModel* /*mesh*/, RichParameterSet* /*parameters*/, StarlabDrawArea* /*drawArea*/);
};

