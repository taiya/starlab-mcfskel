load(starlab)
StarlabTemplate(plugin)
StarlabDepends(surfacemesh)
StarlabExternal(qhull)

# COMMENT OUT FOR QHULL
#CONFIG += matlab

# which library to import?
CONFIG(matlab){
    STARLAB_EXTERNAL += matlab
} else {
    DEFINES += QHULL
    STARLAB_EXTERNAL += qhull
}

HEADERS += voromat.h \
    MatlabVoronoiHelper.h \
    VoronoiHelper.h

SOURCES += voromat.cpp
