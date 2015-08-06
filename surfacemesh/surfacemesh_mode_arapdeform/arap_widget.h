#pragma once
#include "ui_arap_widget.h"
#include "SurfaceMeshPlugins.h"

class surfacemesh_mode_arapdeform;
namespace Ui { class arap_widget; }

class arap_widget: public QWidget{
    Q_OBJECT
public:
    explicit arap_widget(surfacemesh_mode_arapdeform * m = 0);
    ~arap_widget();
private:
    Ui::arap_widget *ui;
    surfacemesh_mode_arapdeform *mode;

public slots:
    void anchorModeChanged(int newMode);
};
