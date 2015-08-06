CONFIG += starlab
STARLAB_TEMPLATE = dynamic

QT *= opengl xml

DEFINES += PARAMETERS_SHARED_EXPORT

HEADERS += \ 
    dynamic_parameters_global.h \
    RichString.h \
    RichParameterWidget.h \
    RichParameterSet.h \
    RichParameter.h \
    RichInt.h \
    RichFloat.h \
    RichFilePath.h \
    RichEnum.h \
    RichColor.h \
    RichBool.h \
    RichAbsPerc.h \
    ParametersFrame.h \
    LineEditWidget.h \  
    RichPar.h \
    RichStringSet.h

SOURCES += \ 
    RichParameterSet.cpp \
    ParametersFrame.cpp
