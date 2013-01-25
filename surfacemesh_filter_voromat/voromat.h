#pragma once
#include <QString>
#include "SurfaceMeshPlugins.h"

/// Parameter names
const QString embedVertices = "embedVertices";
const QString overlayInput  = "overlayInput";
const QString colorizeRadii = "colorizeRadii";
const QString colorizeAngle = "colorizeAngle";

/// Surface property names
const std::string VRADII = "v:radii";
const std::string VANGLE = "v:angle";
const std::string VPOLE  = "v:pole";

class filter : public SurfaceMeshFilterPlugin{
    Q_OBJECT
    Q_INTERFACES(FilterPlugin)

public:
    QString name() { return "Voronoi based MAT"; }
    QString description() { return "Computes voronoi embedding of a surface"; }
    QKeySequence shortcut(){ return QKeySequence(Qt::CTRL + Qt::Key_M); }    
    
    void initParameters(RichParameterSet* pars){
        /// @todo range 0...90deg, for multi-scale medial feature
        // pars->addParam(new RichFloat("Angle",90.0f,"Discarded medial spokes"));
        /// @todo compute it proportionally to average edge length
        // pars->addParam(new RichFloat("Distance",0.01f,"Discarded medial spokes"));
        
        /// Usage parameters
        pars->addParam(new RichBool(embedVertices,false,"Embed Vertices","Should the position of surface vertices be moved to the corresponding medial poles?"));        
        pars->addParam(new RichBool(overlayInput,false,"Overlay input","Should I overlay the input dataset on top of the medial mesh?"));        
        pars->addParam(new RichBool(colorizeRadii,false,"Colorize Medial Radii", "Vertex color is color-mapped to medial radius"));
        pars->addParam(new RichBool(colorizeAngle,false,"Colorize Medial Angle", "Vertex color is color-mapped to medial aperture angle"));
    }
    
    void applyFilter(RichParameterSet* pars);
};
