#include "surfacemesh_filter_flattenlayers.h"
#include "StarlabDrawArea.h"

using namespace SurfaceMesh;

void surfacemesh_filter_flattenlayers::initParameters(RichParameterSet *pars){
    QStringList layers;
    foreach(StarlabModel* model, document()->models())
        if( SurfaceMesh::isA(model) )
            layers << model->name;

    pars->addParam( new RichStringSet("layer_1", layers, "Layer 1", "Merge this layer."));
    pars->addParam( new RichStringSet("layer_2", layers, "Layer 2", "With the one."));
}

void surfacemesh_filter_flattenlayers::applyFilter(RichParameterSet* pars){
    /// Fetch the two models
    QString layer1 = pars->getString("layer_1");
    QString layer2 = pars->getString("layer_2");
    if(layer1==layer2){ qDebug("Don't be silly... give me two different models!"); return; }
    QList<SurfaceMeshModel*> meshes;
    SurfaceMeshModel* mesh1 = SurfaceMesh::safe_cast( document()->getModel(layer1) );
    SurfaceMeshModel* mesh2 = SurfaceMesh::safe_cast( document()->getModel(layer2) );
    meshes << mesh1;
    meshes << mesh2;

    /// Create the merged model
    SurfaceMeshModel* mesh = new SurfaceMeshModel("","Merged Layer");

    /// Collect garbage
    foreach(SurfaceMeshModel* plus_mesh, meshes)
        plus_mesh->garbage_collection();
    
    foreach(SurfaceMeshModel* plus_mesh, meshes){
        /// when we add faces, vertex indexes get offsetted
        size_t offset = mesh->n_vertices();
        
        /// vertex coordinates always exists
        Vector3VertexProperty plus_vpoints = plus_mesh->vertex_coordinates();
        foreach(Vertex v, plus_mesh->vertices())
            mesh->add_vertex( plus_vpoints[v]);
        
        /// Attempt to copy face data
        foreach(Face f, plus_mesh->faces()){
            std::vector<Vertex> vidxs; 
            foreach(Vertex v, plus_mesh->vertices(f))
                vidxs.push_back( Vertex(v.idx()+offset) );
            mesh->add_face(vidxs);
        }         
    }
    
    /// Attempt to copy vertex normals
    if(mesh1->has_vertex_normals() && mesh2->has_vertex_normals()){
        size_t counter = 0;
        Vector3VertexProperty vnormals = mesh->vertex_normals(true);
        foreach(SurfaceMeshModel* plus_mesh, meshes){
            Vector3VertexProperty plus_vnormals = plus_mesh->vertex_normals();
            foreach(Vertex v, plus_mesh->vertices())
                vnormals[Vertex(counter++)] = plus_vnormals[v];              
        }       
    }

    /// Update the document    
    document()->addModel(mesh);
    document()->setSelectedModel(mesh);
    document()->deleteModel(mesh1);
    document()->deleteModel(mesh2);
    
    drawArea()->updateGL();
}

Q_EXPORT_PLUGIN(surfacemesh_filter_flattenlayers)


