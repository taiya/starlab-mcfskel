#pragma once
#include "dynamic_surfacemesh_global.h"

#include <QDebug>
#include <QString>
#include "Model.h"
#include "StarlabException.h"
#include "surface_mesh/Surface_mesh.h"

/// @{ Forward declaration of helpers defined in helper/...
///    This reduces clutter of this class header
class SurfaceMeshForEachVertexHelper;
class SurfaceMeshForEachEdgeHelper;
class SurfaceMeshForEachOneRingEdgesHelper;
class SurfaceMeshForEachFaceHelper;
class SurfaceMeshForEachHalfedgeHelper;
/// @}

class DYNAMIC_SURFACEMESH_EXPORT SurfaceMeshModel : public Model, public Surface_mesh{
    Q_OBJECT
    Q_INTERFACES(Model)
    
    /// @{ Basic Model Implementation
    public:
        SurfaceMeshModel(QString path=QString(), QString name=QString());
        void updateBoundingBox();
        virtual void decorateLayersWidgedItem(QTreeWidgetItem* parent);
    /// @}

    /// @{ Qt foreach helpers
    ///    Example: foreach(Vertex v, m->validVertices()){ ... }
    public:
        using Surface_mesh::halfedges; /// F-you C++
        using Surface_mesh::vertices; /// F-you C++
        using Surface_mesh::faces; /// F-you C++

        SurfaceMeshForEachHalfedgeHelper halfedges();
        SurfaceMeshForEachVertexHelper vertices(); 
        SurfaceMeshForEachEdgeHelper edges();
        SurfaceMeshForEachFaceHelper faces();
        SurfaceMeshForEachOneRingEdgesHelper onering_hedges(Vertex v);
    /// @}

	/// @{ Extra exposed functionality
		void remove_vertex(Vertex v);
	/// @}
};
