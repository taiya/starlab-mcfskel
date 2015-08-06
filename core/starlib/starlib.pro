include($$PWD/../starlab.prf)
StarlabTemplate(sharedlib)

# ---------------------------------------------
# --               EXERNALS                  --
# ---------------------------------------------
include(qglviewer/qglviewer.pri)

# ---------------------------------------------
# --           PARAMETER LIBRARY             --
# ---------------------------------------------
DEPENDPATH += parameters
INCLUDEPATH += parameters
HEADERS += \ 
    parameters/RichString.h \
    parameters/RichParameterWidget.h \
    parameters/RichParameterSet.h \
    parameters/RichParameter.h \
    parameters/RichInt.h \
    parameters/RichFloat.h \
    parameters/RichFilePath.h \
    parameters/RichEnum.h \
    parameters/RichColor.h \
    parameters/RichBool.h \
    parameters/RichAbsPerc.h \
    parameters/RichPar.h \
    parameters/RichStringSet.h \
    parameters/ParametersFrame.h \
    parameters/LineEditWidget.h \  
    Starlab.h \ 
    interfaces/ControllerModePlugin.h \
    StarlabController.h

SOURCES += \ 
    parameters/RichParameterSet.cpp \
    parameters/ParametersFrame.cpp

# ---------------------------------------------
# --                 PLUGINS                 --
# ---------------------------------------------
DEPENDPATH  += interfaces
INCLUDEPATH += interfaces

HEADERS+= \
    interfaces/StarlabPlugin.h \
    interfaces/RenderPlugin.h \
    interfaces/InputOutputPlugin.h \
    interfaces/ProjectInputOutputPlugin.h \
    interfaces/GuiPlugin.h \
    interfaces/FilterPlugin.h \
    interfaces/DecoratePlugin.h \
    interfaces/ModePlugin.h \
    interfaces/ModePluginDockWidget.h

SOURCES += \
    interfaces/StarlabPlugin.cpp

# ---------------------------------------------
# --                  CORE                   --
# ---------------------------------------------
HEADERS += \
    Model.h \
    Document.h \
    StarlabSettings.h \
    StarlabException.h \
    StarlabApplication.h \
    PluginManager.h \
    StarlabDrawArea.h \
    StarlabMainWindow.h \
    RenderObject.h \
    OSQuery.h \
    OpenGLErrorChecker.h \

SOURCES += \
    Model.cpp \
    Document.cpp \
    StarlabSettings.cpp \
    PluginManager.cpp \
    StarlabApplication.cpp \
    StarlabMainWindow.cpp \
    StarlabDrawArea.cpp \
    RenderObject.cpp \
