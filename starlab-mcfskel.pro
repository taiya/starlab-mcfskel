system(qmake -set QMAKEFEATURES $$PWD/starlab)
TEMPLATE = subdirs
CONFIG += ordered

SUBDIRS += voromat
SUBDIRS += mcfskel
#SUBDIRS += solvertest
