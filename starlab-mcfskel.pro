system(qmake -set QMAKEFEATURES $$PWD/starlab.core)
TEMPLATE = subdirs
CONFIG += ordered

# We are loading the full core, you can also compile this independently
SUBDIRS += starlab-core

# We are building only a portion of the surfacemesh submodule
SUBDIRS += surfacemesh/surfacemesh
SUBDIRS += surfacemesh/surfacemesh_filter_normalize
SUBDIRS += surfacemesh/surfacemesh_render_smooth
SUBDIRS += surfacemesh/surfacemesh_render_flatwire
SUBDIRS += surfacemesh/surfacemesh_render_transparent

# Add some components to deal with curve-skeletons
# (This was a horrible idea.. we should have just re-used the surfacemesh)
SUBDIRS += curveskel
SUBDIRS += curveskel_io_cg
SUBDIRS += curveskel_io_skc
SUBDIRS += curveskel_render_lines
SUBDIRS += curveskel_filter_resample
SUBDIRS += curveskel_filter_compare

# Add the components of this plugin
# @note we are using a custom "io_obj" plugin
SUBDIRS += surfacemesh_io_obj
SUBDIRS += surfacemesh_filter_remesher
SUBDIRS += surfacemesh_filter_to_skeleton
#SUBDIRS += surfacemesh_filter_voromat
#SUBDIRS += surfacemesh_filter_mcfskel

