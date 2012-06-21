CONFIG += starlab 
STARLAB_TEMPLATE += plugin 
STARLAB_DEPENDS += surfacemesh_model
STARLAB_EXTERNAL += eigen-3.1.0-rc1

HEADERS += mcfskel.h \
    TopologyJanitor.h \
    ContractionHelper.h \
    SurfaceAreaHelper.h \
    PoleAttractorHelper.h \
    MatlabLaplacianHelper.h

SOURCES += mcfskel.cpp
