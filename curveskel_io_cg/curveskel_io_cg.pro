CONFIG += starlab
STARLAB_TEMPLATE = plugin
STARLAB_DEPENDS += ../curveskel

HEADERS += curveskel_io_cg.h
SOURCES += curveskel_io_cg.cpp 

# Windows warnings
win32: DEFINES += _CRT_SECURE_NO_WARNINGS
