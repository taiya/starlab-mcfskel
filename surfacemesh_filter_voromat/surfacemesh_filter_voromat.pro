load($$[STARLAB])
load($$[SURFACEMESH])
load($$[QHULL])
StarlabTemplate(plugin)

HEADERS += voromat.h

#---- QHULL VERSION
HEADERS += QhullVoronoiHelper.h
SOURCES += voromat_qhull.cpp

#---- UNCOMMENT for MATLAB version
# STARLAB_EXTERNAL += matlab
# SOURCES += voromat_matlab.cpp
# HEADERS += MatlabVoronoiHelper.h
