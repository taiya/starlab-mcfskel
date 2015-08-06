include($$[STARLAB])
include($$[SURFACEMESH])
StarlabTemplate(plugin)

HEADERS = \
    plugin.h
SOURCES = \
    plugin.cpp
RESOURCES = \
    plugin.qrc

OTHER_FILES += \
    transparent_mesh.png
