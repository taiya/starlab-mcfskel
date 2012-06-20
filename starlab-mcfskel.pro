system(qmake -set QMAKEFEATURES $$PWD/starlab)
TEMPLATE = subdirs
CONFIG += ordered

SUBDIRS += voromat_qhull
SUBDIRS += mcfskel
#SUBDIRS += solvertest
