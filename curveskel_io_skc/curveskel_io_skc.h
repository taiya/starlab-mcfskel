#pragma once
#include "CurveskelPlugins.h"
 
class curveskel_io_skc : public CurveskelInputOutputPlugin{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "curveskel_io_skc.plugin.starlab")
    Q_INTERFACES(InputOutputPlugin)
    
public:
    QString name(){ return "[Curveskel] Day's Skeleton Curve (*.skc)"; }
    Starlab::Model* open(QString path);
    void save(CurveskelModel*, QString);
};
