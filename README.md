# Mean Curvature Skeletons
(Automatically exported from https://code.google.com/p/starlab-mcfskel)

The source & compiled binaries from the paper "Mean Curvature Skeletons". The paper is available in the download section. In the downloads you will also find pre-compiled binaries for Window, OSX and Ubuntu.

```
@article{taglia_sgp12,
title={Mean Curvature Skeletons},
author={Andrea Tagliasacchi and Ibraheem Alhashim and Matt Olson and Hao Zhang},
booktitle={Computer Graphics Forum (Proc. of the Symposium on Geometry Processing)},
year={2012}}
```

## Downloads
- [Paper PDF](Downloads/taglia_sgp12.pdf)
- [Paper BibTex](Downloads/taglia_sgp12.txt)
- [Presentation (Keynote)](taglia_sgp12.key)
- [Demo Executable (Windows)](Downloads/mcfskel-v1.1-win32.zip)
- [Demo Executable (OSX)](Downloads/Starlab.dmg)

## Checking out the source code 
```
git clone https://code.google.com/p/starlab-mcfskel
```

Then open the file *mcfskel.pro* in `QtCreator` and perform *qmake* & *build*

This is a set of plugins for the starlab environment specific to this project. The set contains:
```
remesher                    generates an approximately uniform mesh
voromat                     generates a medial manifold by projecting a mesh onto the voronoi poles
mcfskel                     mean curvature skeletonization (with medial guidance)
surfacemesh_to_skeleton     a simple plugin to convert contracted meshes into curve-skeletons 
surfacemesh_io_obj          I/O of obj files containing medial information
skeleton_resample           finely resamples a curve-skeleton (used in comparisons)
skeleton_compare            compares euclidean distance between two skeletons
```

## Usage 
A typical usage is to load the mesh, apply a re-meshing operation, apply the *voromat* plugin without the embedding option, then start the skeletonization process (MCF steps). The resulting mesh can be collapsed into simple curves by applying the *short_ecollapse* plugin. The result can be saved to 'cg' file format.

## Gallery
![](https://lh6.googleusercontent.com/-jA6ubOslwZE/T_laLl8Ki0I/AAAAAAAAnI0/b3Yc_eMJgxg/s800/code_gallery.png)

## Example Video (Click to play)

[![Example](http://img.youtube.com/vi/gs5R2RhngVA/0.jpg)](http://www.youtube.com/watch?v=gs5R2RhngVA)

## Google Summer of Code
Xiang Gao has re-implemented this project in CGAL while participating in the CGAL Google Summer of Code 2013
<div align="center"> <img src="/Downloads/gsoc.png" width="30%"/> </div>
