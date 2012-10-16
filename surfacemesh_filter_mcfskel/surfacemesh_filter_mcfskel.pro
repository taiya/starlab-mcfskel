load(starlab)
StarlabTemplate(plugin)
StarlabDepends(surfacemesh)
StarlabExternal(eigen)
StarlabExternal(cholmod)

# Uncomment to use matlab as a solver instead of eigen
# CONFIG += matlab 
CONFIG(matlab){
    DEFINES += USE_MATLAB
    STARLAB_EXTERNAL += matlab 
}

DEPENDPATH += $$PWD

HEADERS += \
    Skelcollapse.h \
    TopologyJanitor.h \
    TopologyJanitor_ClosestPole.h \
    MatlabContractionHelper.h \
    EigenContractionHelper.h \
    CotangentLaplacianHelper.h \
    MeanValueLaplacianHelper.h \
    Logfile.h

SOURCES += \  
    Skelcollapse.cpp \
    Logfile.cpp


