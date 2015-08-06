include($$PWD/../starlab.prf)
StarlabTemplate(console)

TARGET = starterm

HEADERS += \
    QCommandLine.h \
    CmdLineParser.h

SOURCES += main.cpp \
    QCommandLine.cpp \
    CmdLineParser.cpp
