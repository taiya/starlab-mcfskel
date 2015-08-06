# this project generates the application bundle Starlab.app
include($$PWD/../starlab.prf)
StarlabTemplate(appbundle)

TARGET = starlab
win32:DESTDIR = $$EXECUTABLEPATH

HEADERS += FileOpenEater.h \
    StarlabApplicationGUI.h
SOURCES += main.cpp

#------------- BASIC CONFIG ---------------
# NOTE: QtCreator bug, $$[DSTPATH] wouldn't produce correct run configuration
QT *= opengl
RESOURCES = starlab.qrc
RC_FILE = starlab.rc
ICON = images/starlab.icns

#--- Setup DYLD path, so we can run starlab everywhere.... BUT THIS DOES NOT WORK :(
#macx:QMAKE_POST_LINK="export DYLD_LIBRARY_PATH=$$[BLDPATH]/distrib/starlab.app/Contents/MacOS/"
#--- and neither does this...
#runstarlabeverywhere.target = runstarlabeverywhere
#runstarlabeverywhere.commands = export DYLD_LIBRARY_PATH=$$[BLDPATH]/distrib/starlab.app/Contents/MacOS/
#QMAKE_EXTRA_TARGETS += runstarlabeverywhere
#POST_TARGETDEPS += runstarlabeverywhere
