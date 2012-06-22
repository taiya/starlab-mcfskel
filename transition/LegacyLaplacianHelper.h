#pragma once
#include "SurfaceMeshModel.h"
#include "SurfaceMeshTypes.h"
#include "StarlabException.h"
#include "SurfaceMeshHelper.h"

class LaplacianHelper : public virtual SurfaceMeshHelper{
protected:
    Vector3VertexProperty laplace;
  
public:
    LaplacianHelper(SurfaceMeshModel* mesh) : SurfaceMeshHelper(mesh){
        laplace = mesh->vertex_property<Point>("v:laplace",Vector3(0));        
    }
    virtual void applyLaplacianIteration(float damping=0.5f){
        foreach(Vertex v, mesh->vertices())
            points[v] += damping * laplace[v];
    }
    Vector3VertexProperty computeLaplacianVectors(const string property="h:weight"){
        laplace = mesh->vertex_property<Vector3>(property);
        foreach(Vertex v, mesh->vertices()){
            laplace[v] = 0;
            Vector3 p = points[v];
            /// Up to this point it is the centroid
            foreach(Halfedge hi, mesh->onering_hedges(v)){
                Vertex vi = mesh->to_vertex(hi);
                Vector3 pi = points[vi];
                laplace[v] += pi;
            }
            laplace[v] /= (Scalar) mesh->valence(v);
            /// Now it is the comb laplacian
            laplace[v] = laplace[v]-p;
        }
        return laplace;
    }
};

class MeanValueLaplacianHelper : public virtual LaplacianHelper{
protected:
    Surface_mesh::Halfedge_property<Scalar> hweight; /// NULL
      
public:
    MeanValueLaplacianHelper(SurfaceMeshModel* mesh) : SurfaceMeshHelper(mesh), LaplacianHelper(mesh){}
    
    void computeMeanValueHalfEdgeWeights(Scalar edgelength_eps=0.0, char* property="h:weight"){
        hweight = mesh->add_halfedge_property<Scalar>(property);
        
        /// Compute mean value weight on halfedges
        foreach(Edge e, mesh->edges()){
            Halfedge h0 = mesh->halfedge(e,0);
            Halfedge h1 = mesh->halfedge(e,1);
            hweight[h0] = halfEdgeWeight(h0,edgelength_eps);
            hweight[h1] = halfEdgeWeight(h1,edgelength_eps);
        }
    }
    Vector3VertexProperty computeLaplacianVectors(bool autonormalized=true){
        foreach(Vertex v,mesh->vertices())
            laplace[v] = meanValueLaplacianVector(v,autonormalized);
        return laplace;
    }
    
    /// Requies: call to voronoiArea done before
    /// unstable has been scaled by area, so it has same direction of the stable one
    /// consequently all I need to do is compare vector norms
    Vector3VertexProperty computeAreaNormalizedStableLaplacianVectors(){
        Scalar normalizer=1; /// Originally no change
        foreach(Vertex vit,mesh->vertices()){
            Vector3 stableLaplacian = meanValueLaplacianVector(vit,true);
            Vector3 unstableLaplacian = meanValueLaplacianVector(vit,false) / varea[vit];
            laplace[vit] = unstableLaplacian;
            
            Scalar stableOffLength = stableLaplacian.norm();
            Scalar unstableOffLength = unstableLaplacian.norm();
            Scalar currNormalizer = unstableOffLength/stableOffLength;
            if( currNormalizer>normalizer )
                normalizer = currNormalizer;
        }
        foreach(Vertex vit,mesh->vertices())
            laplace[vit] /= normalizer;
        
        return laplace;
    }

protected:
    Scalar halfEdgeWeight(Halfedge h, Scalar edgelength_eps){        
        Vertex   v0 = mesh->from_vertex(h);
        Vertex   v1 = mesh->to_vertex(h);
        Point    p0 = points[v0];
        Point    p1 = points[v1];
        Scalar   edgelength = (p0-p1).norm();
        
        Halfedge h_prev = mesh->opposite_halfedge( mesh->prev_halfedge(h) );
        Halfedge h_next = mesh->next_halfedge( mesh->opposite_halfedge(h) );
        
        Vertex v_prev = mesh->to_vertex(h_prev);
        Vertex v_next = mesh->to_vertex(h_next);
        Point  p_prev = points[v_prev];
        Point  p_next = points[v_next];
        
        if(mesh->is_boundary(h) || mesh->is_boundary(h_prev) || mesh->is_boundary(h_next))
            throw StarlabException("Boundary-less not supported!!!");
        
        Vector3 d_prev = (p_prev-p0).normalize();
        Vector3 d_curr = (p1    -p0).normalize();
        Vector3 d_next = (p_next-p0).normalize();
        Scalar dot_prev = qBound( -.99, dot(d_curr,d_prev), +.99 );
        Scalar dot_next = qBound( -.99, dot(d_curr,d_next), +.99 );
        Scalar alpha_prev = acos( dot_prev );
        Scalar alpha_next = acos( dot_next );   
        Scalar weight = ( tan(alpha_prev/2) + tan(alpha_next/2) ) / ( edgelength + edgelength_eps );
        // qDebug() << dot(p0,p_prev) << dot(p0,p_next) << weight << " edge: " <<  edgelength;        
        
        if(isnan(weight) || isinf(weight)){
            // drawArea->drawSegment(p0,p1,2);
            // drawArea->drawPoint(p0,2);
            // drawArea->drawPoint(p1,2);
            qDebug() << edgelength << alpha_prev << alpha_next;
            if(isnan(weight)) qDebug() << "NAN";
            if(isinf(weight)) qDebug() << "INF";
            throw StarlabException("Invalid weight");
        }
        return weight;            
    }
    

    
    /// Requires: manifold
    /// Requires: hweight
    Vector3 meanValueLaplacianVector(Vertex v, bool autonormalize=true){
        Vector3 laplace(0);
        Scalar weights=0;
        foreach(Halfedge h, mesh->onering_hedges(v)){
            Point p0 = points[mesh->from_vertex(h)];
            Point p1 = points[mesh->to_vertex(h)];
            laplace += hweight[h]*(p1-p0);
            weights += hweight[h];
        }
        
        Vector3 retval;
        /// Stable normalization
        if(autonormalize)
            retval = laplace/weights;
            
        if( isnan(retval[0]) || isinf(retval[0]) )
            throw StarlabException("Not a number!!");
        
        /// Return normalized vector
        return retval;
    }
};

class  CotangentLaplacianHelper : public virtual LaplacianHelper{    
protected:
    Surface_mesh::Edge_property<Scalar>     eweight;  /// NULL
    Surface_mesh::Vertex_property<Vector3> laplace;  /// NULL

public:
    CotangentLaplacianHelper(SurfaceMeshModel* mesh) : SurfaceMeshHelper(mesh), LaplacianHelper(mesh){}
        
    ScalarEdgeProperty computeCotangentEdgeWeights(const string property="e:weight"){
        eweight = mesh->add_edge_property<Scalar>(property);
        foreach(Edge e, mesh->edges())
            eweight[e] = cotangentLaplacianWeight(e);
        return eweight;
    }
    Vector3VertexProperty computeLaplacianVectors(const string property="v:laplace", bool autonormalize=true){
        laplace = mesh->add_vertex_property<Vector3>(property);
        foreach(Vertex v, mesh->vertices())
            laplace[v] = cotangentLaplacianVector(v,autonormalize);
        return laplace;
    }
    
protected:
    Scalar cotangentLaplacianWeight(Edge eit){
        Surface_mesh::Vertex_property<Point> points = mesh->vertex_property<Point>("v:point");
       
        bool secure = true;
        
        Scalar lb(-0.999), ub(0.999);        
        Scalar w  = 0.0;
        Surface_mesh::Halfedge h0 = mesh->halfedge(eit, 0);
        Surface_mesh::Vertex   v0 = mesh->to_vertex(h0);
        Point p0 = points[v0];
        
        Surface_mesh::Halfedge h1 = mesh->halfedge(eit, 1);
        Surface_mesh::Vertex   v1 = mesh->to_vertex(h1);
        Point p1 = points[v1];
        
        if(mesh->is_boundary(h0) || mesh->is_boundary(h1))
            throw StarlabException("Not supported here!!!");
            
        if (!mesh->is_boundary(h0))
        {
            Point p2 = points[mesh->to_vertex(mesh->next_halfedge(h0))];
            Point d0 = (p0 - p2).normalize();
            Point d1 = (p1 - p2).normalize();
            Scalar c  = dot(d0,d1);
            if(secure) if (c<lb) c=lb; else if (c>ub) c=ub;
            w += 1.0 / tan(acos(c));
        }
        
        if (!mesh->is_boundary(h1))
        {
            Point p2 = points[mesh->to_vertex(mesh->next_halfedge(h1))];
            Point d0 = (p0 - p2).normalize();
            Point d1 = (p1 - p2).normalize();
            Scalar c  = dot(d0,d1);
            if(secure) if (c<lb) c=lb; else if (c>ub) c=ub;
            w += 1.0 / tan(acos(c));
        }
        
        // force weights to be non-negative for higher robustness
        if(secure) if (w < 0.0) w = 0.0;
        return w;
    }
    
    Vector3 cotangentLaplacianVector(Vertex vit, bool autoNormalize){
        Point l=0;
        if( autoNormalize ){
            Scalar w=0;
            if(!mesh->is_boundary(vit)){
                foreach(Halfedge hvit, mesh->onering_hedges(vit)){
                    Vertex v = mesh->to_vertex(hvit);
                    Edge e = mesh->edge(hvit);
                    l += eweight[e] * (points[v] - points[vit]);
                    w += eweight[e];
                }                
                l /= w;
            }
        } else {
            if(!mesh->is_boundary(vit)){
                foreach(Halfedge hvit, mesh->onering_hedges(vit)){
                    Vertex v = mesh->to_vertex(hvit);
                    Edge e = mesh->edge(hvit);
                    l += eweight[e] * (points[v] - points[vit]) / (2*varea[vit]);
                }                
            }
        }
        return l;
    }    
    
#if 0
    void cotangentLaplacianSmooth(SurfaceMeshModel* mesh, int iters){
        for(int i=0;i<iters;i++){
            foreach(Edge eit,mesh->edges())
                eweight[eit] = cotangentLaplacianWeight(mesh,eit);
            foreach(Vertex vit,mesh->vertices())
                laplace[vit] = cotangentLaplacianVector(mesh,vit,true);
            foreach(Vertex v, mesh->vertices())
                points[v] += .5f * laplace[v];
        }
    }
#endif    
};
