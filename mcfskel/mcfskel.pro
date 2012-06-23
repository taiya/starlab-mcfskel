CONFIG += starlab
STARLAB_TEMPLATE += plugin 
STARLAB_DEPENDS += surfacemesh_model
STARLAB_EXTERNAL += matlab cholmod eigen

DEPENDPATH += $$PWD
#DEFINES += USE_MATLAB

#--- What solver would you like to use?
HEADERS += \
    Skelcollapse.h \
    TopologyJanitor.h \
    TopologyJanitor_ClosestPole.h \
    MatlabContractionHelper.h \
    EigenContractionHelper.h \
    CotangentLaplacianHelper.h \
    MeanValueLaplacianHelper.h

SOURCES += \  
    Skelcollapse.cpp


