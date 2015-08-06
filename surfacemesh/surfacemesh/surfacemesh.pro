include($$[STARLAB])
StarlabTemplate(sharedlib)
 
# Expose this to the global build
system(qmake -set SURFACEMESH $$PWD/surfacemesh.prf)
OTHER_FILES = surfacemesh.prf

# THIS IS FOR OUR MODEL WRAPPER
HEADERS += SurfaceMeshModel.h \
    dynamic_surfacemesh_global.h \
    SurfaceMeshHelper.h \
    SurfaceMeshPlugins.h \
    helpers/ColorMap.h \
    helpers/LaplacianHelper.h \
    helpers/CurvatureEstimationHelper.h \
    helpers/MatlabSurfaceMeshHelper.h \
    helpers/SurfaceMeshQForEachHelpers.h \
    helpers/ColorizeHelper.h \
    helpers/DoUndoHelper.h \
    helpers/LocalAnalysisHelper.h \
    helpers/StatisticsHelper.h \
    helpers/SurfaceMeshNormalsHelper.h \
    iterators/StdIterators.h \ 
    SurfaceMesh/NoiseGenerator.h \    
    helpers/FaceBarycenterHelper.h
 
SOURCES += SurfaceMeshModel.cpp

# THESE ARE THE ONES OF THE REAL LIBRARY
HEADERS +=
HEADERS += geometry/Quadric.h
HEADERS += surface_mesh/properties.h
HEADERS += surface_mesh/Surface_mesh.h
HEADERS += surface_mesh/IO.h
HEADERS += surface_mesh/gl_wrappers.h

SOURCES += surface_mesh/IO.cpp
SOURCES += surface_mesh/IO_stl.cpp
SOURCES += surface_mesh/IO_obj.cpp
SOURCES += surface_mesh/IO_off.cpp
SOURCES += surface_mesh/Surface_mesh.cpp

# Windows specific
win32: DEFINES += _CRT_SECURE_NO_WARNINGS # disable 'unsafe' warnings
