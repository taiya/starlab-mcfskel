#pragma once

#include <QDebug>
#include <QString>

#include "global.h"
#include "Model.h"
#include "CurveskelTypes.h"

namespace CurveskelTypes
{
    /// @{ Forward declaration of helpers defined in helper/...
    ///    This reduces clutter of this class header
    class CurveskelForEachVertexHelper;
    class CurveskelForEachEdgeHelper;
    /// @}

    class EXPORT CurveskelModel : public Starlab::Model, public CurveskelTypes::MyWingedMesh{
        Q_OBJECT
        Q_INTERFACES(Starlab::Model)

    public:
        CurveskelModel(QString path=QString(), QString name=QString());
        void updateBoundingBox();

        /// @{ Qt foreach helpers
        public:
            CurveskelForEachVertexHelper vertices();
            CurveskelForEachEdgeHelper edges();
        /// @}

        std::set<Vertex> junctions();

		std::set<Vertex> adjacent_set(Vertex v);
		Vertex other_vertex(Edge e, Vertex v);
    };
}
