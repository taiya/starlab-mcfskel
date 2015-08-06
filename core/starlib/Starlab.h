#pragma once
#include "Eigen/Dense"
#include "StarlabException.h"

/** 
 * @defgroup starcore Starlab Core Classes
 * @brief The classes around which starlab is constructed.
 */ 

namespace Starlab{
    class Model;
    class Document;
    class DrawArea;
    class Application;
    class MainWindow;
    class Settings;
    class PluginManager;    
    class Controller;
}

/// @todo Should RichParameterSet be inserted into Starlab::?
class RichParameterSet;


/// In Starlab, Math is managed by EIGEN. EIGEN types are registered by Qt
/// in by something like "qRegisterMetaType<BBox3>("BBox3");" before being 
/// able to be used in Signal/Slots. The registration is done in StarlabApplication().

namespace Starlab{
    typedef double Scalar;
    typedef Eigen::Vector3d Vector3;
    typedef Eigen::Hyperplane<Scalar,3> Plane3;
    typedef Eigen::ParametrizedLine<Scalar,3> Ray3;
    typedef Eigen::AlignedBox3d BBox3;
}

/// Basic render plugins names
#include <QString>
namespace SHADING{
    const QString FLAT   = "Flat Shading";
    const QString SMOOTH = "Smooth Shading";
}
