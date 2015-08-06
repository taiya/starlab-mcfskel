load($$[STARLAB])
StarlabTemplate(plugin)

# these is the core stuff
HEADERS += gui_python.h
SOURCES += gui_python.cpp
RESOURCES += gui_python.qrc


# now include all the sources from the extracted lib
INCLUDEPATH += pythonqt
include(python.prf)
include(pythonqt/pythonqt.pri)
