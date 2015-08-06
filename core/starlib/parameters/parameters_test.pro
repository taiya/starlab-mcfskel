QT *= opengl xml gui
TARGET = test
CONFIG += console
CONFIG -= app_bundle

DEFINES += DEBUG_CODE
   
HEADERS += \
    RichString.h \
    RichParameterWidget.h \
    RichParameterSet.h \
    RichParameter.h \
    RichPar.h \
    RichInt.h \
    RichFloat.h \
    RichFilePath.h \
    RichEnum.h \
    RichColor.h \
    RichBool.h \
    RichAbsPerc.h \
    ParametersFrame.h \
    LineEditWidget.h \
    dynamic_parameters_global.h

SOURCES += \
    RichParameterSet.cpp \
    ParametersFrame.cpp \
    main.cpp





















