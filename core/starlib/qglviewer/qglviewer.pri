QT += opengl xml
# linux does not have GLU included in += opengl
unix:!mac: LIBS += -lGLU

DEPENDPATH  += qglviewer
INCLUDEPATH += qglviewer

win32:DEFINES += CREATE_QGLVIEWER_DLL

# because we are not including vectorial headers
DEFINES += NO_VECTORIAL_RENDER

HEADERS = qglviewer/qglviewer.h \
          qglviewer/camera.h \
          qglviewer/manipulatedFrame.h \
          qglviewer/manipulatedCameraFrame.h \
          qglviewer/frame.h \
          qglviewer/constraint.h \
          qglviewer/keyFrameInterpolator.h \
          qglviewer/mouseGrabber.h \
          qglviewer/quaternion.h \
          qglviewer/vec.h \
          qglviewer/domUtils.h \
          qglviewer/config.h

SOURCES = qglviewer/qglviewer.cpp \
          qglviewer/camera.cpp \
          qglviewer/manipulatedFrame.cpp \
          qglviewer/manipulatedCameraFrame.cpp \
          qglviewer/frame.cpp \
          qglviewer/saveSnapshot.cpp \
          qglviewer/constraint.cpp \
          qglviewer/keyFrameInterpolator.cpp \
          qglviewer/mouseGrabber.cpp \
          qglviewer/quaternion.cpp \
          qglviewer/vec.cpp

FORMS *= qglviewer/ImageInterface.ui
