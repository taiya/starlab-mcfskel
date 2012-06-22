CONFIG += starlab
STARLAB_TEMPLATE += plugin 
STARLAB_DEPENDS += surfacemesh_model

DEPENDPATH  *= $$PWD
INCLUDEPATH *= $$PWD

#--- What solver would you like to use?
CONFIG += matlab 
# CONFIG += eigen

# which library to import?
CONFIG(matlab): STARLAB_EXTERNAL += matlab
CONFIG(eigen):  STARLAB_EXTERNAL += taucs eigen

HEADERS += \
    Skelcollapse.h \
    SurfaceAreaHelper.h \
    TopologyJanitor.h \
    TopologyJanitor_ClosestPole.h \
    ContractionHelper.h \
    MatlabContractionHelper.h

SOURCES += \
    Skelcollapse.cpp


