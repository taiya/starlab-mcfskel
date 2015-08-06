#include "surfacemesh_filter_geoheat.h"
#include "StarlabDrawArea.h"
#include "GeoDrawObjects.h"

void surfacemesh_filter_geoheat::initParameters(RichParameterSet *pars)
{
    pars->addParam(new RichInt("sourceVertex", 0, "Source vertex"));
    pars->addParam(new RichBool("visualizeDistance", true, "Visualize Distance"));
}

void surfacemesh_filter_geoheat::applyFilter(RichParameterSet *pars)
{
    QSet<Vertex> src;
    if(pars)
        src.insert(Vertex(pars->getInt("sourceVertex")));
    else
    {
        // Source points are tagged on mesh vertices
        BoolVertexProperty src_points = mesh()->get_vertex_property<bool>("v:geo_src_points");
        if(!src_points.is_valid()) return;
        foreach(Vertex v, mesh()->vertices())
            if(src_points[v]) src.insert(v);
    }

    if(!h) h = new GeoHeatHelper(mesh());

    ScalarVertexProperty distance = h->getUniformDistance(src);

    // Visualize distance on vertices
    if(pars && pars->getBool("visualizeDistance"))
    {
        drawArea()->deleteAllRenderObjects();

        PointSoup * ps = new PointSoup;
        Vector3VertexProperty points = h->getVector3VertexProperty(VPOINT);

        foreach(Vertex v, mesh()->vertices())
            ps->addPoint( points[v], qtJetColorMap(1.0 - distance[v]) );
        drawArea()->addRenderObject(ps);
    }
}

Q_EXPORT_PLUGIN(surfacemesh_filter_geoheat)
