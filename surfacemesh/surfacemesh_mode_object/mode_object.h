#pragma once
#include "ModePlugin.h"

class mode_object : public ModePlugin{
    Q_OBJECT
    Q_INTERFACES(ModePlugin)
    
/// @{ StarlabPlugin
    QString name() { return "Object Mode"; }
    QIcon icon(){ return QIcon(":/icons/mode_object.png"); }
    bool isApplicable(){  return true; }
/// @}
    
/// @{ ModePlugin
    void create(){}
    void destroy(){}
    bool keyReleaseEvent(QKeyEvent*);
/// @}    

/// @{ Local stuff
    QString command;   
    void draw();
public slots:
    void execute(QString command);
/// @}
    
/// @{ fully disable mouse interaction
    virtual bool mousePressEvent        (QMouseEvent* ) { return true; }
    virtual bool mouseMoveEvent         (QMouseEvent* ) { return true; }
    virtual bool mouseReleaseEvent      (QMouseEvent* ) { return true; }
    virtual bool keyPressEvent          (QKeyEvent*   ) { return true; }
    virtual bool tabletEvent            (QTabletEvent*) { return true; }
    virtual bool wheelEvent             (QWheelEvent* ) { return true; }
    virtual bool mouseDoubleClickEvent  (QMouseEvent* ) { return true; }
/// @} 
    
};
