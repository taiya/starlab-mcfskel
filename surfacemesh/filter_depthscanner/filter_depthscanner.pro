include($$[STARLAB])
include($$[OCTREE])
include($$[SURFACEMESH])
StarlabTemplate(plugin)

HEADERS += SurfaceMesh/DepthTriangulator.h

HEADERS += filter_depthscan.h
SOURCES += filter_depthscan.cpp
