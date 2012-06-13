CONFIG += starlab 
STARLAB_TEMPLATE += plugin 
STARLAB_DEPENDS += surfacemesh_model

HEADERS += voromat.h \
    MatlabVoronoiHelper.h
SOURCES += voromat.cpp
 
include(matlab.pri)
