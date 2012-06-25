CONFIG += starlab 
STARLAB_TEMPLATE += plugin 
STARLAB_DEPENDS *= surfacemesh_model
STARLAB_DEPENDS *= skeleton_model

HEADERS += surfacemesh_filter_to_skeleton.h
SOURCES += surfacemesh_filter_to_skeleton.cpp
 
