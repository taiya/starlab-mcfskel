#include "IsotropicRemesher.h"
#include "SurfaceMeshHelper.h"
#include "StarlabDrawArea.h"
#include <QElapsedTimer>

#include "MathHelper.h"

void IsotropicRemesher::remesh(double targetEdgeLength, int numIterations, bool isProjectSurface, bool isKeepShortEdges)
{
    QElapsedTimer timer; timer.start();

    const double low  = (4.0 / 5.0) * targetEdgeLength;
    const double high = (4.0 / 3.0) * targetEdgeLength;

    // Copy original mesh
    SurfaceMeshModel* copy = new SurfaceMeshModel(mesh()->path,"original");
    if(isProjectSurface) *(Surface_mesh*)copy = *(Surface_mesh*)mesh();

    for(int i = 0; i < numIterations; i++)
    {
        splitLongEdges(high);
        collapseShortEdges(low, high, isKeepShortEdges);
        equalizeValences();
        tangentialRelaxation();
        if(isProjectSurface) projectToSurface( copy );
    }

    qDebug() << "[IsotropicRemesher] time spent " << timer.elapsed() << " ms." ;
}

/// performs edge splits until all edges are shorter than the threshold
void IsotropicRemesher::splitLongEdges(double maxEdgeLength )
{
    const double maxEdgeLengthSqr = maxEdgeLength * maxEdgeLength;

    SurfaceMeshModel::Edge_iterator e_it;
    SurfaceMeshModel::Edge_iterator e_end = mesh()->edges_end();

    // iterate over all edges
    for (e_it = mesh()->edges_begin(); e_it != e_end; ++e_it){
        const SurfaceMeshModel::Halfedge & hh = mesh()->halfedge( e_it, 0 );

        const SurfaceMeshModel::Vertex & v0 = mesh()->from_vertex(hh);
        const SurfaceMeshModel::Vertex & v1 = mesh()->to_vertex(hh);

        Vector3 vec = points[v1] - points[v0];

        // edge to long?
        if ( vec.squaredNorm() > maxEdgeLengthSqr ){

            const Vector3 midPoint = points[v0] + ( 0.5 * vec );

            // split at midpoint
            SurfaceMeshModel::Vertex vh = mesh()->add_vertex( midPoint );

            bool hadFeature = efeature[e_it];

            mesh()->split(e_it, vh);

            if ( hadFeature )
            {
                foreach(Halfedge e, mesh()->onering_hedges(vh))
                {
                    if ( mesh()->to_vertex(e) == v0 || mesh()->to_vertex(e) == v1 )
                    {
                        efeature[mesh()->edge(e)] = true;
                    }
                }
            }
        }
    }
}

/// collapse edges shorter than minEdgeLength if collapsing doesn't result in new edge longer than maxEdgeLength
void IsotropicRemesher::collapseShortEdges(const double _minEdgeLength, const double _maxEdgeLength, bool isKeepShortEdges )
{
    const double _minEdgeLengthSqr = _minEdgeLength * _minEdgeLength;
    const double _maxEdgeLengthSqr = _maxEdgeLength * _maxEdgeLength;

    //add checked property
    BoolEdgeProperty checked = mesh()->edge_property< bool >("e:checked", false);

    SurfaceMeshModel::Edge_iterator e_it;
    SurfaceMeshModel::Edge_iterator e_end = mesh()->edges_end();

    bool finished = false;

    while( !finished ){

        finished = true;

        for (e_it = mesh()->edges_begin(); e_it != mesh()->edges_end() ; ++e_it){

            if ( checked[e_it] )
                continue;

            checked[e_it] = true;

            const SurfaceMeshModel::Halfedge & hh = mesh()->halfedge( e_it, 0 );

            const SurfaceMeshModel::Vertex & v0 = mesh()->from_vertex(hh);
            const SurfaceMeshModel::Vertex & v1 = mesh()->to_vertex(hh);

            const Vector3 vec = points[v1] - points[v0];

            const double edgeLength = vec.squaredNorm();

            // Keep originally short edges, if requested
            bool hadFeature = efeature[e_it];
            if ( isKeepShortEdges && hadFeature ) continue;

            // edge too short but don't try to collapse edges that have length 0
            if ( (edgeLength < _minEdgeLengthSqr) && (edgeLength > std::numeric_limits<double>::epsilon()) ){

                //check if the collapse is ok
                const Vector3 & B = points[v1];

                bool collapse_ok = true;

                foreach( Halfedge hvit, mesh()->onering_hedges(v0) )
                {
                    double d = (B - points[ mesh()->to_vertex(hvit) ]).squaredNorm();

                    if ( d > _maxEdgeLengthSqr || mesh()->is_boundary( mesh()->edge( hvit ) ) || efeature[mesh()->edge(hvit)] )
                    {
                        collapse_ok = false;
                        break;
                    }
                }

                if( collapse_ok && mesh()->is_collapse_ok(hh) ) {
                    mesh()->collapse( hh );

                    finished = false;
                }
            }
        }
    }

    mesh()->remove_edge_property(checked);

    mesh()->garbage_collection();
}

void IsotropicRemesher::equalizeValences(  )
{
    SurfaceMeshModel::Edge_iterator e_it;
    SurfaceMeshModel::Edge_iterator e_end = mesh()->edges_end();

    for (e_it = mesh()->edges_begin(); e_it != e_end; ++e_it){

        if ( !mesh()->is_flip_ok(e_it) ) continue;
        if ( efeature[e_it] ) continue;

        const SurfaceMeshModel::Halfedge & h0 = mesh()->halfedge( e_it, 0 );
        const SurfaceMeshModel::Halfedge & h1 = mesh()->halfedge( e_it, 1 );

        if (h0.is_valid() && h1.is_valid())
        {
            if (mesh()->face(h0).is_valid() && mesh()->face(h1).is_valid()){
                //get vertices of corresponding faces
                const SurfaceMeshModel::Vertex & a = mesh()->to_vertex(h0);
                const SurfaceMeshModel::Vertex & b = mesh()->to_vertex(h1);
                const SurfaceMeshModel::Vertex & c = mesh()->to_vertex(mesh()->next_halfedge(h0));
                const SurfaceMeshModel::Vertex & d = mesh()->to_vertex(mesh()->next_halfedge(h1));

                const int deviation_pre =  abs((int)(mesh()->valence(a) - targetValence(a)))
                        +abs((int)(mesh()->valence(b) - targetValence(b)))
                        +abs((int)(mesh()->valence(c) - targetValence(c)))
                        +abs((int)(mesh()->valence(d) - targetValence(d)));
                mesh()->flip(e_it);

                const int deviation_post = abs((int)(mesh()->valence(a) - targetValence(a)))
                        +abs((int)(mesh()->valence(b) - targetValence(b)))
                        +abs((int)(mesh()->valence(c) - targetValence(c)))
                        +abs((int)(mesh()->valence(d) - targetValence(d)));

                if (deviation_pre <= deviation_post)
                    mesh()->flip(e_it);
            }
        }
    }
}

///returns 4 for boundary vertices and 6 otherwise
inline int IsotropicRemesher::targetValence(const SurfaceMeshModel::Vertex& _vh ){
    if (isBoundary(_vh))
        return 4;
    else
        return 6;
}

inline bool IsotropicRemesher::isBoundary(const SurfaceMeshModel::Vertex& _vh )
{
	foreach( Halfedge hvit, mesh()->onering_hedges(_vh) )
	{
		if ( mesh()->is_boundary( mesh()->edge( hvit ) ) )
			return true;
	}
    return false;
}

inline bool IsotropicRemesher::isFeature(const SurfaceMeshModel::Vertex& _vh )
{
	foreach( Halfedge hvit, mesh()->onering_hedges(_vh) )
	{
		if(efeature[mesh()->edge(hvit)])
			return true;
	}

    return false;
}

void IsotropicRemesher::tangentialRelaxation(  )
{
    mesh()->update_face_normals();
    mesh()->update_vertex_normals();

    Vector3VertexProperty q = mesh()->vertex_property<Vector3>("v:q");
    Vector3VertexProperty normal = mesh()->vertex_property<Vector3>(VNORMAL);

    SurfaceMeshModel::Vertex_iterator v_it;
    SurfaceMeshModel::Vertex_iterator v_end = mesh()->vertices_end();

    //first compute barycenters
    for (v_it = mesh()->vertices_begin(); v_it != v_end; ++v_it){

        Vector3 tmp(0,0,0);
        uint N = 0;

        foreach( Halfedge hvit, mesh()->onering_hedges(v_it) )
        {
            tmp += points[ mesh()->to_vertex(hvit) ];
            N++;
        }

        if (N > 0)
            tmp /= (double) N;

        q[v_it] = tmp;
    }

    //move to new position
    for (v_it = mesh()->vertices_begin(); v_it != v_end; ++v_it)
    {
        if ( !isBoundary(v_it) && !isFeature(v_it) )
        {
            //Vector3 newPos = q[v_it] + (dot(normal[v_it], (points[v_it] - q[v_it]) ) * normal[v_it]);
            points[v_it] = q[v_it];
        }
    }

    mesh()->remove_vertex_property(q);
}

Vector3 IsotropicRemesher::findNearestPoint(SurfaceMeshModel * original_mesh, const Vector3& _point, SurfaceMeshModel::Face& _fh, double* _dbest)
{
    Vector3VertexProperty orig_points = original_mesh->vertex_property<Vector3>( VPOINT );

	double fc = original_mesh->bbox().diagonal().norm() * 2;
    Vector3  p_best = Vector3(fc,fc,fc) + Vector3(original_mesh->bbox().center());
    SurfaceMeshModel::Scalar d_best = (_point - p_best).squaredNorm();
    SurfaceMeshModel::Face fh_best;

    // exhaustive search
    foreach(Face f, original_mesh->faces())
    {
        Surface_mesh::Vertex_around_face_circulator cfv_it = original_mesh->vertices(f);

        // Assume triangular
        const Vector3& pt0 = orig_points[   cfv_it];
        const Vector3& pt1 = orig_points[ ++cfv_it];
        const Vector3& pt2 = orig_points[ ++cfv_it];

        Vector3 ptn = _point;

        //SurfaceMeshModel::Scalar d = distPointTriangleSquared( _point, pt0, pt1, pt2, ptn );
        SurfaceMeshModel::Scalar d = ClosestPointTriangle( _point, pt0, pt1, pt2, ptn );

        if( d < d_best)
        {
            d_best = d;
            p_best = ptn;

            fh_best = f;
        }
    }

    // return face
    _fh = fh_best;

    // return distance
    if(_dbest) *_dbest = sqrt(d_best);

    return p_best;
}

void IsotropicRemesher::projectToSurface(SurfaceMeshModel * orginal_mesh )
{
    SurfaceMeshModel::Vertex_iterator v_it;
    SurfaceMeshModel::Vertex_iterator v_end = mesh()->vertices_end();

    for (v_it = mesh()->vertices_begin(); v_it != v_end; ++v_it)
    {
        if (isBoundary(v_it)) continue;
        if ( isFeature(v_it)) continue;

        Vector3 p = points[v_it];

        SurfaceMeshModel::Face fhNear;
        double distance;

        Vector3 pNear = findNearestPoint(orginal_mesh, p, fhNear, &distance);

        points[v_it] = pNear;
    }
}

void IsotropicRemesher::initParameters(RichParameterSet* parameters)
{
    Scalar edgelength_TH = 0.02 * mesh()->bbox().diagonal().norm();

    parameters->addParam(new RichFloat("edgelength_TH",edgelength_TH,"Target edge length", "By default it's 2% of bbox diagonal"));

    parameters->addParam(new RichInt("num_iters",10,"#Iterations", "The number of applied iterations"));
	parameters->addParam(new RichBool("sharp_features",true,"Preserve sharp features", ""));
	parameters->addParam(new RichFloat("sharp_features_angle",44.0,"Sharp angle max", ""));
    parameters->addParam(new RichBool("project_surface",true,"Project to original", ""));
    parameters->addParam(new RichBool("keep_shortedges",false,"Keep short edges", ""));

    /// We are doing re-meshing, interested in seeing the mesh edges...
    drawArea()->setRenderer(mesh(),"Flat Wire");
}

void IsotropicRemesher::applyFilter(RichParameterSet* pars)
{
    Scalar longest_edge_length  = pars->getFloat("edgelength_TH");
    Counter num_split_iters     = pars->getInt("num_iters");

	points = mesh()->vertex_property<Vector3>( VPOINT );

	// Prepare for sharp features
	efeature = mesh()->edge_property<bool>("e:feature", false);
	if(pars->getBool("sharp_features")){
		double angleDeg = pars->getFloat("sharp_features_angle");
		double angleThreshold = deg_to_rad(angleDeg);

		foreach(Edge e, mesh()->edges())
		{
			if (abs(calc_dihedral_angle(*mesh(), mesh()->halfedge(e,0))) > angleThreshold)
				efeature[e] = true;
		}
	}

    // Prepare for short edges on original mesh
    if(pars->getBool("keep_shortedges")){
        foreach(Edge e, mesh()->edges()){
            const SurfaceMeshModel::Halfedge & hh = mesh()->halfedge( e, 0 );
            const SurfaceMeshModel::Vertex & v0 = mesh()->from_vertex(hh);
            const SurfaceMeshModel::Vertex & v1 = mesh()->to_vertex(hh);
            const Vector3 vec = points[v1] - points[v0];

            if (vec.norm() <= longest_edge_length)
                efeature[e] = true;
        }
    }

    /// Perform refinement
    this->remesh(longest_edge_length, num_split_iters, pars->getBool("project_surface"), pars->getBool("keep_shortedges"));

	// Clean up
	mesh()->remove_edge_property(efeature);
}

Q_EXPORT_PLUGIN(IsotropicRemesher)
