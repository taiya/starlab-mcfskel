CONFIG += starlab
STARLAB_TEMPLATE += plugin 
STARLAB_DEPENDS += surfacemesh_model

DEPENDPATH  *= $$PWD
INCLUDEPATH *= $$PWD

#--- What solver would you like to use?
CONFIG += matlab 
#CONFIG += cholmod

# which library to import?
CONFIG(matlab):     STARLAB_EXTERNAL += matlab
CONFIG(cholmod):    STARLAB_EXTERNAL += cholmod eigen

HEADERS += \
    Skelcollapse.h \
    SurfaceAreaHelper.h \
    TopologyJanitor.h \
    TopologyJanitor_ClosestPole.h \
    MatlabContractionHelper.h \
    EigenContractionHelper.h

SOURCES += \
    Skelcollapse.cpp


