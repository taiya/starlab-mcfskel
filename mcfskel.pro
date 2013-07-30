#--- Setup the global paths
TEMPLATE = subdirs
CONFIG += ordered

# We are loading the full core, you can also compile this independently
SUBDIRS += core

# We have imported only a PORTION of the full surfacemesh
SUBDIRS += surfacemesh/surfacemesh
SUBDIRS += surfacemesh/surfacemesh_io_off
SUBDIRS += surfacemesh/surfacemesh_io_obj 
SUBDIRS += surfacemesh/surfacemesh_filter_normalize
SUBDIRS += surfacemesh/surfacemesh_render_smooth
SUBDIRS += surfacemesh/surfacemesh_render_flatwire
SUBDIRS += surfacemesh/surfacemesh_render_transparent

# Add some components to deal with curve-skeletons
SUBDIRS += curveskel
SUBDIRS += curveskel_io_cg
SUBDIRS += curveskel_io_skc
SUBDIRS += curveskel_render_lines
SUBDIRS += curveskel_filter_resample
SUBDIRS += curveskel_filter_compare

# And the ones specific to this project
SUBDIRS += surfacemesh_filter_to_skeleton
SUBDIRS += surfacemesh_filter_voromat
SUBDIRS += surfacemesh_filter_mcfskel
SUBDIRS += surfacemesh_filter_remesher #< botsch's remesher
#SUBDIRS += surfacemesh_filter_isotropic_remesher #< Better remesher
#SUBDIRS += surfacemesh_io_obj #< customized to read/write poles to/from file!!
