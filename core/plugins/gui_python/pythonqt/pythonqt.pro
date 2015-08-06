load($$[STARLAB])
StarlabTemplate(sharedlib)
 
DEFINES +=  PYTHONQT_EXPORTS

# Expose this to the global build
system(qmake -set PYTHONQT \"$$PWD/pythonqt.prf\")
OTHER_FILES = pythonqt.prf

include(python.prf)  

HEADERS +=                    \
  PythonQt.h                  \
  PythonQtStdDecorators.h     \
  PythonQtClassInfo.h         \
  PythonQtImporter.h          \
  PythonQtObjectPtr.h         \
  PythonQtSignal.h            \
  PythonQtSlot.h              \
  PythonQtStdIn.h             \
  PythonQtStdOut.h            \
  PythonQtMisc.h              \
  PythonQtMethodInfo.h        \
  PythonQtImportFileInterface.h \
  PythonQtConversion.h        \
  PythonQtSignalReceiver.h    \
  PythonQtInstanceWrapper.h   \
  PythonQtClassWrapper.h \
  PythonQtCppWrapperFactory.h \
  PythonQtQFileImporter.h     \
  PythonQtQFileImporter.h     \
  PythonQtVariants.h          \
  gui/PythonQtScriptingConsole.h    \
  PythonQtSystem.h
  
SOURCES +=                    \
  PythonQtStdDecorators.cpp   \
  PythonQt.cpp                \
  PythonQtClassInfo.cpp       \
  PythonQtImporter.cpp        \
  PythonQtObjectPtr.cpp       \
  PythonQtStdIn.cpp           \
  PythonQtStdOut.cpp          \
  PythonQtSignal.cpp          \
  PythonQtSlot.cpp            \
  PythonQtMisc.cpp            \
  PythonQtMethodInfo.cpp      \
  PythonQtConversion.cpp      \
  PythonQtSignalReceiver.cpp  \
  PythonQtInstanceWrapper.cpp \
  PythonQtQFileImporter.cpp   \
  PythonQtClassWrapper.cpp    \
  gui/PythonQtScriptingConsole.cpp \
 

# This was modified from:
#include(../generated_cpp/com_trolltech_qt_core_builtin/com_trolltech_qt_core_builtin.pri)
HEADERS += \
           com_trolltech_qt_core_builtin0.h 
SOURCES += \
           com_trolltech_qt_core_builtin0.cpp \
           com_trolltech_qt_core_builtin_init.cpp

#include(../generated_cpp/com_trolltech_qt_gui_builtin/com_trolltech_qt_gui_builtin.pri
HEADERS += \
           com_trolltech_qt_gui_builtin0.h \

SOURCES += \
           com_trolltech_qt_gui_builtin0.cpp \
           com_trolltech_qt_gui_builtin_init.cpp

