system(qmake -set QMAKEFEATURES $$PWD/starlab)
TEMPLATE = subdirs
CONFIG += ordered

SUBDIRS += remesher
# SUBDIRS += voromat_qhull
SUBDIRS += voromat
SUBDIRS += mcfskel
SUBDIRS += surfacemesh_io_obj


#--- OLD VERSION!!
#SUBDIRS += skelcollapse

#--- Transition version
SUBDIRS += transition
