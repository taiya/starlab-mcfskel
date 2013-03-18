#pragma once
#include "CurveskelPlugins.h"
 
class curveskel_io_cg : public CurveskelInputOutputPlugin{
    Q_OBJECT
    Q_INTERFACES(InputOutputPlugin)
    
public:
    QString name(){ return "[Curveskel] Curve Graph (*.cg)"; }
    Starlab::Model* open(QString path);
    void save(CurveskelModel*, QString);
};
