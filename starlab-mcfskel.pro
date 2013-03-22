#--- Setup the global paths
TEMPLATE = subdirs
CONFIG += ordered

# We are loading the full core, you can also compile this independently
SUBDIRS += starlab-core

# We have imported only a PORTION of the full surfacemesh
SUBDIRS += starlab-surfacemesh/surfacemesh
SUBDIRS += starlab-surfacemesh/surfacemesh_filter_normalize
SUBDIRS += starlab-surfacemesh/surfacemesh_render_smooth
SUBDIRS += starlab-surfacemesh/surfacemesh_render_flatwire
SUBDIRS += starlab-surfacemesh/surfacemesh_render_transparent

# Add some components to deal with curve-skeletons
SUBDIRS += curveskel
SUBDIRS += curveskel_io_cg
SUBDIRS += curveskel_io_skc
SUBDIRS += curveskel_render_lines
SUBDIRS += curveskel_filter_resample
SUBDIRS += curveskel_filter_compare

# And the ones specific to this project
SUBDIRS += surfacemesh_io_obj #< customized!!
SUBDIRS += surfacemesh_filter_to_skeleton
SUBDIRS += surfacemesh_filter_voromat
SUBDIRS += surfacemesh_filter_mcfskel
#SUBDIRS += surfacemesh_filter_remesher
SUBDIRS += surfacemesh_filter_isotropic_remesher #< Better remesher
