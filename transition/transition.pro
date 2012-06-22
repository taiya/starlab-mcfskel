CONFIG += starlab
STARLAB_TEMPLATE += plugin 
STARLAB_DEPENDS += surfacemesh_model
STARLAB_EXTERNAL += matlab

DEPENDPATH  *= $$PWD
INCLUDEPATH *= $$PWD

HEADERS += \
    Skelcollapse.h \
    SurfaceAreaHelper.h \
    TopologyJanitor.h \
    TopologyJanitor_ClosestPole.h \
    ContractionHelper.h \
    PoleAttractorHelper.h

SOURCES += \
    Skelcollapse.cpp


