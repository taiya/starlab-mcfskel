CONFIG += starlab 
STARLAB_TEMPLATE += plugin 
STARLAB_DEPENDS += surfacemesh_model
STARLAB_EXTERNAL += qhull

HEADERS += voromat.h VoronoiHelper.h
SOURCES += voromat.cpp

#include(matlab.pri)
