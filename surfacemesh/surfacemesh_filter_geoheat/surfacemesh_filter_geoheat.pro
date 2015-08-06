include($$[STARLAB])
include($$[SURFACEMESH])
include($$[CHOLMOD])
StarlabTemplate(plugin)

HEADERS += surfacemesh_filter_geoheat.h
SOURCES += surfacemesh_filter_geoheat.cpp

HEADERS += GeoHeatHelper.h
HEADERS += GeoDrawObjects.h
