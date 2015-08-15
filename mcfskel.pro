#--- Setup the global paths
TEMPLATE = subdirs
CONFIG += ordered

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
