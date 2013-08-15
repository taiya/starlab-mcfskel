#pragma once
#include "SurfaceMeshPlugins.h"
#include "SurfaceMeshHelper.h"

class IsotropicRemesher : public SurfaceMeshFilterPlugin{
    Q_OBJECT
    Q_INTERFACES(FilterPlugin)

public:
    QString name() { return "Isotropic Remesher"; }
    QString description() { return "Remeshes the surface by applying tangential flow, edge splits, collapses and flips"; }
    void applyFilter(RichParameterSet*);
    void initParameters(RichParameterSet* parameters);

    Vector3VertexProperty points;
    BoolEdgeProperty efeature;

private:
    void remesh(double targetEdgeLength, int numIterations, bool isProjectSurface, bool isKeepShortEdges = false);
    void splitLongEdges(double maxEdgeLength);
    void collapseShortEdges(const double _minEdgeLength, const double _maxEdgeLength, bool isKeepShortEdges);
    void equalizeValences();
	void tangentialRelaxation();
    int targetValence(const SurfaceMeshModel::Vertex &_vh);
    bool isBoundary(const SurfaceMeshModel::Vertex &_vh);
    bool isFeature(const SurfaceMeshModel::Vertex &_vh);

    void projectToSurface(SurfaceMeshModel * orginal_mesh);
    Vector3 findNearestPoint(SurfaceMeshModel * orginal_mesh, const Vector3& _point, SurfaceMeshModel::Face& _fh ,double* _dbest);
};
