load($$[STARLAB])
load($$[SURFACEMESH])
StarlabTemplate(plugin)

# THIS PLUGIN COULD BE IMPROVED IN TWO WAYS:
# 1) selection should change coloring of **faces**
# 2) at every mouse movement we are re-rendering the whole name buffer.
#    this is imposed by qglviewer but could be improved!!!
# 
# 
# MORE RESOURCES:
# http://www.glprogramming.com/red/chapter13.html
#
#
# TODO: right now it only "paints" selection. It could be 
# changed to paint color or scalar fields as well!!
#
HEADERS += mode_facepaint.h
SOURCES += mode_facepaint.cpp
RESOURCES += mode_facepaint.qrc
