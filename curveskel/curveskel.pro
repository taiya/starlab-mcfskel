load(starlab)
StarlabTemplate(sharedlib)

# Dependencies
DEFINES += EXPORTFLAG
OTHER_FILES += *.prf

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
