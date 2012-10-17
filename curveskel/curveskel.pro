load($$[STARLAB])
StarlabTemplate(sharedlib)

# expose library to qmake
system(qmake -set CURVESKEL $$PWD/curveskel)
OTHER_FILES += curveskel.prf

# Dependencies
DEFINES += EXPORTFLAG

HEADERS += \
    global.h \
    Vector.h \
    WingedgeMesh.h \
    CurveskelModel.h \
    CurveskelPlugins.h \
    CurveskelTypes.h \
    CurveskelHelper.h \
    CurveskelQForEach.h

SOURCES += CurveskelModel.cpp
