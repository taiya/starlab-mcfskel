system(qmake -set QMAKEFEATURES $$PWD/starlab)
TEMPLATE = subdirs
CONFIG += ordered

SUBDIRS += remesher
SUBDIRS += voromat
SUBDIRS += mcfskel
SUBDIRS += surfacemesh_io_obj
 
#--- DISABLED
# SUBDIRS += skelcollapse
# SUBDIRS += voromat_qhull
