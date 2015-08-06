include($$PWD/../../starlab.prf)
StarlabTemplate(plugin)

HEADERS = \
    plugin.h

SOURCES = plugin.cpp
RESOURCES = plugin.qrc
OTHER_FILES += \
    bbox.png
