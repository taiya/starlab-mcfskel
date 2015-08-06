#pragma once
#include <QMenu>
#include "interfaces/GuiPlugin.h"
#include "StarlabMainWindow.h"
#include "StarlabDrawArea.h"

class gui_view : public GuiPlugin{
    Q_OBJECT
    Q_INTERFACES(GuiPlugin)

/// @{ splits load in pieces
public:
    void load();
private:
    void load_clearRenderObjects();
    void load_viewfrom();
    void load_fullscreen();
    void load_copypasteviewmatrix();
    void load_toggleCameraProjection();
/// @}    
};
