#pragma once
#include "CurveskelPlugins.h"

class plugin : public CurveskelRenderPlugin{
    Q_OBJECT
    Q_INTERFACES(RenderPlugin)

public:
    QString name() { return "Skeleton Edges"; }
    QIcon icon(){ return QIcon(":/sticks.png"); }
    Renderer* instance();

protected:
    virtual bool isDefault() { return true; }
};
