# The "object" plugin is supposed to emulate the one of blender.
# 
# Scripted actions: "r x +10" 
# Roto/Translation of selected models with mouse
# etc...

load($$[STARLAB])
load($$[EIGEN])
load($$[SURFACEMESH])
StarlabTemplate(plugin)

HEADERS += mode_object.h 
SOURCES += mode_object.cpp
RESOURCES += mode_object.qrc
OTHER_FILES += mode_object.png
