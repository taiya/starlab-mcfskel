CONFIG += starlab 
STARLAB_TEMPLATE += plugin 
STARLAB_DEPENDS += surfacemesh_model
STARLAB_EXTERNAL += eigen-3.1.0-rc1

# Load the external solver
load($$PWD/../external/cholmod/cholmod.prf)
OTHER_FILES += $$PWD/../external/cholmod/cholmod.prf

include(matlab.pri)

HEADERS += mcfskel.h \
    TopologyJanitor.h \
    ContractionHelper.h \
    SurfaceAreaHelper.h \
    MatlabLaplacianHelper.h \
    PoleAttractorHelper.h \
    LegacyTopologyJanitor.h

SOURCES += mcfskel.cpp
