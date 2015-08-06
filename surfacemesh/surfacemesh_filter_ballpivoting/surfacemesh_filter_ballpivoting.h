#pragma once
#include "SurfaceMeshPlugins.h"
class surfacemesh_filter_ballpivoting : public SurfaceMeshFilterPlugin{
    Q_OBJECT
    Q_INTERFACES(FilterPlugin)

public:
    QString name() { return "Surface Reconstruction: Ball Pivoting"; }
    QString description() { return "The ball-pivoting algorithm for surface reconstruction. "
                                    " Bernardini F., Mittleman J., Rushmeier H., Silva C., Taubin G. "
                                    " IEEE TVCG 1999"; }
								
	void initParameters(RichParameterSet* pars);
    void applyFilter(RichParameterSet*);
};
