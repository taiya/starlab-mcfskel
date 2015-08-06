#include "plugin.h"
Q_EXPORT_PLUGIN(plugin)

#include "stdlib.h" //RAND_MAX
#include "NanoKDTree3.h"
#include "StarlabDrawArea.h"
#include "SurfaceMesh/NoiseGenerator.h"

using namespace SurfaceMesh;



void plugin::initParameters(RichParameterSet* pars){
    pars->addParam(new RichFloat("mean", 0.1, "Hole size mean "));
    pars->addParam(new RichFloat("var", 0.0, "Size Variance"));
    pars->addParam(new RichInt("num", 10, "Attempts"));
}

void plugin::applyFilter(RichParameterSet* pars) {
    SurfaceMesh::VertexCoordinatesIterator vci(mesh());
    NanoKDTree3<Vector3> kd(vci.begin(), vci.end());
    mesh()->garbage_collection(); /// Clean up    
    
    Scalar mean = pars->getFloat("mean");
    Scalar stddev = std::sqrt( pars->getFloat("var") );
    Integer num = pars->getInt("num");
    
    Vector3VertexProperty points = mesh()->vertex_coordinates();

    /// What vertices were deleted
    std::vector<bool> keepv( mesh()->n_vertices(), true );
    
    // qsrand(0);
    for(int i=0; i<num; i++){
        int idx = ( double(qrand()) / RAND_MAX ) * (mesh()->n_vertices()-1);
        double radius = NoiseGenerator::randn(mean,stddev);
        std::vector<size_t> idxs = kd.ball_search( points[Vertex(idx)], radius );
        for(uint j=0; j<idxs.size(); j++)
            keepv[ idxs[j] ] = false;
    }
    
    /// Copy the kept vertices in the target 
    SurfaceMeshModel* newmesh = new SurfaceMeshModel(mesh()->path, mesh()->name);
    foreach(Vertex v, mesh()->vertices())
        if(keepv[v.idx()])
            newmesh->add_vertex(points[v]);
            
    /// By the end of this process, we need to know what are the indexes of the 
    /// new vertices in the new mesh, so to be able to transfer faces
    IntegerVertexProperty newidx = mesh()->add_vertex_property<Integer>("newidx",-1);
    int curridx = 0;
    foreach(Vertex v, mesh()->vertices())
        if(keepv[v.idx()])
            newidx[v] = curridx++;

    foreach(Face f, mesh()->faces()){
        /// Don't add faces that have invalid
        bool skip = false;
        std::vector<Vertex> vtsIdxs;
        foreach(Vertex v, mesh()->vertices(f)){
            vtsIdxs.push_back( Vertex(newidx[v]) );
            if( keepv[v.idx()]==false )
                skip = true;
        }
        if(skip) continue;
        
        /// Finally add face
        newmesh->add_face(vtsIdxs);
    }
    
    
    /// Swap old with new model
    document()->addModel(newmesh);
    document()->deleteModel(model());
}


QString plugin::description(){
    return "Chews the hell out of your point cloud";
}
