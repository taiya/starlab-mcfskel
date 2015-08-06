include($$[STARLAB])
include($$[SURFACEMESH])
StarlabTemplate(plugin)

DEFINES += TODO_WIREFRAME_VBUFFER

HEADERS = \
    plugin.h
SOURCES = \
    plugin.cpp
RESOURCES = \
    plugin.qrc

OTHER_FILES += \
    wireframe.png
