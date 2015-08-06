#pragma once
#include "StarlabMainWindow.h"
#include "SurfaceMeshPlugins.h"
#include "ControllerModePlugin.h"
#include <QMouseEvent>

class mode_facepaint : public ModePlugin{
    Q_OBJECT
    Q_INTERFACES(ModePlugin)

    QIcon icon(){ return QIcon(":/icons/mode_facepaint.png"); }
    bool isApplicable(){ return true; }

private:
    SurfaceMeshModel* mesh;
    BoolFaceProperty fselect;
    Vector3FaceProperty fbarycenter;
    Vector3VertexProperty vpoints;
    
    
public:
    /// Functions part of the EditPlugin system
    void create();
    void destroy(){}
    
    Qt::MouseButton _button; ///< Qt::NoButton
    bool mouseReleaseEvent(QMouseEvent*);
    bool mousePressEvent(QMouseEvent*);
    bool mouseMoveEvent(QMouseEvent*);
    void drawWithNames();
    void decorate();
};
