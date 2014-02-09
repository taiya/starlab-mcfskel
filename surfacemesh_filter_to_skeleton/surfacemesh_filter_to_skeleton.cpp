#include"surfacemesh_filter_to_skeleton.h"
#include "CurveskelModel.h"
#include "CurveskelHelper.h"
#include "MyPriorityQueue.h"

void surfacemesh_filter_to_skeleton::applyFilter(RichParameterSet* /*parameters*/){
    SurfaceMeshModel* model = mesh();
    model->garbage_collection();
    
    /// Create a new "skeletal" model and add it to document
    CurveskelTypes::CurveskelModel* skel = new CurveskelTypes::CurveskelModel("","skeleton");
    document()->addModel(skel);
    

    /// 0) modify WindedgeMesh.h if you need anything below
    /// 1) read through triangles and fill in the wingedge data structure

    // vertices
    Surface_mesh::Vertex_property<SurfaceMeshModel::Point> points = model->get_vertex_property<SurfaceMeshModel::Point>("v:point");
    for (Surface_mesh::Vertex_iterator vit = model->vertices_begin(); vit!=model->vertices_end(); ++vit)
    {
        SurfaceMeshModel::Point p = points[vit];
        skel->add_vertex(CurveskelTypes::Vector3(p[0], p[1], p[2]));
    }

    // faces
    for (Surface_mesh::Face_iterator fit = model->faces_begin(); fit!=model->faces_end(); ++fit)
    {
        Surface_mesh::Vertex_around_face_circulator fvit = model->vertices(fit), fvend=fvit;
        std::vector<CurveskelTypes::Vertex> vertices;

        do {
            int vi = Surface_mesh::Vertex(fvit).idx();
            vertices.push_back(CurveskelTypes::Vertex(vi));
        }
        while (++fvit != fvend);

        skel->add_face(vertices);
    }

	skel->print_stats();

    /// 2) perform sorted edge collapse
	CurveskelTypes::CurveskelHelper sh(skel);
	CurveskelTypes::ScalarEdgeProperty elen = sh.computeEdgeLengths();
	
	// Add to priority queue
	CurveskelTypes::MyPriorityQueue queue(skel);
	foreach(CurveskelTypes::Edge edge, skel->edges())
		queue.insert(edge, elen[edge]);

	// This will be used to position collapsed vertices
	CurveskelTypes::CurveskelModel::Vertex_property<CurveskelTypes::Point> skel_points = skel->vertex_property<CurveskelTypes::Point>("v:point");
	CurveskelTypes::CurveskelModel::Vertex_property< std::set<CurveskelTypes::Vertex> > vrecord = skel->vertex_property< std::set<CurveskelTypes::Vertex> >("v:collapse-from");

	// First add yourself to the set
	foreach(CurveskelTypes::Vertex v, skel->vertices())
		vrecord[v].insert(v);

	int counter = 0;

	/// Collapse cycle
	while (!queue.empty()){
		//qDebug() << "counter: " << counter;

		/// Retrieve shortest edge
		CurveskelTypes::CurveskelModel::Edge e = queue.pop();

		/// Make sure edge was not already dealt with by previous collapses
		if(!skel->has_faces(e) || skel->is_deleted(e) || !skel->is_valid(e))
			continue;

		CurveskelTypes::CurveskelModel::Vertex v1 = skel->vertex(e, 0); // 'v1' will be deleted
		CurveskelTypes::CurveskelModel::Vertex v2 = skel->vertex(e, 1);

		/// Do collapse
		skel->collapse(e);

		/// record collapsed vertex
		vrecord[v2].insert(v1);

		// carry its records too 
		if(vrecord[v1].size()) 
			vrecord[v2].insert(vrecord[v1].begin(), vrecord[v1].end());

		/// Re-position target vertex to midpoint [look at code after loop]
		//skel_points[v2] = (skel_points[v1] + skel_points[v2]) / 2;

		/// Update length of edges incident to remaining vertex
		CurveskelTypes::CurveskelModel::Edge_around_vertex eit (skel, v2);

		while(!eit.end())
		{
			CurveskelTypes::CurveskelModel::Edge edge = eit;

			double newLength = skel->edge_length(edge);

			// If edge still in queue, update its position
			if(queue.has(edge))
				queue.update(edge, newLength);
			
			++eit;
		}
		
		//qDebug() << "size " << queue.set.size();
		counter++;
	}

	// Move to centroid of collapsed vertices
	for(uint vi = 0; vi < model->n_vertices(); vi++)
	{
		CurveskelTypes::CurveskelModel::Vertex v(vi);

		// Only active vertices, since we didn't garbage collect
		if(!skel->is_deleted(v))
		{
			// If its not collapsed keep it at old position
			SurfaceMeshModel::Point p = points[SurfaceMeshModel::Vertex(vi)];
			skel_points[v] = CurveskelTypes::Vector3(p[0], p[1], p[2]);

			// Else, assign to centroid 
			if(vrecord[v].size())
			{
				CurveskelTypes::Vector3 center(0,0,0);

				// Center of corresponding vertices
				foreach(CurveskelTypes::Vertex v, vrecord[v])
				{
					SurfaceMeshModel::Point p = points[SurfaceMeshModel::Vertex(v.idx())];
					center += CurveskelTypes::Vector3(p[0], p[1], p[2]);
				}

				center /= vrecord[v].size();

				skel_points[v] = center;
			}
		}
	}

	/// now, delete the items that have been marked to be deleted
    skel->garbage_collection();
	skel->print_stats();
}
