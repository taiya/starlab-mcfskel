#include"surfacemesh_filter_to_skeleton.h"
#include "SkeletonModel.h"
#include "SkeletonHelper.h"
#include "MyPriorityQueue.h"

void surfacemesh_filter_to_skeleton::applyFilter(Document* document, RichParameterSet* /*parameters*/, StarlabDrawArea* /*drawArea*/){
    SurfaceMeshModel* model = qobject_cast<SurfaceMeshModel*>(document->selectedModel());
    model->garbage_collection();
    
    /// Create a new "skeletal" model and add it to document
    SkeletonModel* skel = new SkeletonModel("","skeleton");
    document->addModel(skel);
    

    /// 0) modify WindedgeMesh.h if you need anything below
    /// 1) read through triangles and fill in the wingedge data structure

    // vertices
    Surface_mesh::Vertex_property<SurfaceMeshModel::Point> points = model->get_vertex_property<SurfaceMeshModel::Point>("v:point");
    for (Surface_mesh::Vertex_iterator vit = model->vertices_begin(); vit!=model->vertices_end(); ++vit)
    {
        SurfaceMeshModel::Point p = points[vit];
        skel->add_vertex(SkeletonTypes::Vector3(p[0], p[1], p[2]));
    }

    // faces
    for (Surface_mesh::Face_iterator fit = model->faces_begin(); fit!=model->faces_end(); ++fit)
    {
        Surface_mesh::Vertex_around_face_circulator fvit = model->vertices(fit), fvend=fvit;
        std::vector<SkeletonTypes::Vertex> vertices;

        do {
            int vi = Surface_mesh::Vertex(fvit).idx();
            vertices.push_back(SkeletonTypes::Vertex(vi));
        }
        while (++fvit != fvend);

        skel->add_face(vertices);
    }

	skel->print_stats();

    /// 2) perform sorted edge collapse
	SkeletonTypes::SkeletonHelper sh(skel);
	SkeletonTypes::ScalarEdgeProperty elen = sh.computeEdgeLengths();
	
	// Add to priority queue
	SkeletonTypes::MyPriorityQueue queue(skel);
	foreach(SkeletonTypes::Edge edge, skel->edges())
		queue.insert(edge, elen[edge]);

	// This will be used to position collapsed vertices
	SkeletonModel::Vertex_property<SkeletonTypes::Point> skel_points = skel->vertex_property<SkeletonTypes::Point>("v:point");
	SkeletonModel::Vertex_property< std::set<SkeletonTypes::Vertex> > vrecord = skel->vertex_property< std::set<SkeletonTypes::Vertex> >("v:collapse-from");

	// First add yourself to the set
	foreach(SkeletonTypes::Vertex v, skel->vertices())
		vrecord[v].insert(v);

	int counter = 0;

	/// Collapse cycle
	while (!queue.empty()){
		//qDebug() << "counter: " << counter;

		/// Retrieve shortest edge
		SkeletonModel::Edge e = queue.pop();

		/// Make sure edge was not already dealt with by previous collapses
		if(!skel->has_faces(e) || skel->is_deleted(e) || !skel->is_valid(e))
			continue;

		SkeletonModel::Vertex v1 = skel->vertex(e, 0); // 'v1' will be deleted
		SkeletonModel::Vertex v2 = skel->vertex(e, 1);

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
		SkeletonModel::Edge_around_vertex eit (skel, v2);

		while(!eit.end())
		{
			SkeletonModel::Edge edge = eit;

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
		SkeletonModel::Vertex v(vi);

		// Only active vertices, since we didn't garbage collect
		if(!skel->is_deleted(v))
		{
			// If its not collapsed keep it at old position
			SurfaceMeshModel::Point p = points[SurfaceMeshModel::Vertex(vi)];
			skel_points[v] = SkeletonTypes::Vector3(p[0], p[1], p[2]);

			// Else, assign to centroid 
			if(vrecord[v].size())
			{
				SkeletonTypes::Vector3 center(0,0,0);

				// Center of corresponding vertices
				foreach(SkeletonTypes::Vertex v, vrecord[v])
				{
					SurfaceMeshModel::Point p = points[SurfaceMeshModel::Vertex(v.idx())];
					center += SkeletonTypes::Vector3(p[0], p[1], p[2]);
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

Q_EXPORT_PLUGIN(surfacemesh_filter_to_skeleton)
