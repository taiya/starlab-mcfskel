#pragma once
#include "SurfaceMeshHelper.h"
class MeanValueLaplacianHelper : public virtual SurfaceMeshHelper{
public:
    MeanValueLaplacianHelper(SurfaceMeshModel* mesh) : SurfaceMeshHelper(mesh){}
    
    ScalarHalfedgeProperty computeMeanValueHalfEdgeWeights(Scalar edgelength_eps=0.0, char* property="h:weight"){
        ScalarHalfedgeProperty hweight = mesh->add_halfedge_property<Scalar>(property);
        
        /// Compute mean value weight on halfedges
        foreach(Edge e, mesh->edges()){
            Halfedge h0 = mesh->halfedge(e,0);
            Halfedge h1 = mesh->halfedge(e,1);
            hweight[h0] = halfEdgeWeight(h0,edgelength_eps);
            hweight[h1] = halfEdgeWeight(h1,edgelength_eps);
        }
        return hweight;
    }
    Vector3VertexProperty computeLaplacianVectors(ScalarHalfedgeProperty hweight, const string property="v:laplace", bool autonormalized=true){
        Vector3VertexProperty laplace = getVector3VertexProperty(property);
        foreach(Vertex v,mesh->vertices())
            laplace[v] = meanValueLaplacianVector(hweight,v,autonormalized);
        return laplace;
    }
    
    /// Requies: call to voronoiArea done before
    /// unstable has been scaled by area, so it has same direction of the stable one
    /// consequently all I need to do is compare vector norms
    Vector3VertexProperty computeAreaNormalizedStableLaplacianVectors(ScalarHalfedgeProperty hweight, const string property="v:laplace"){
        Vector3VertexProperty laplace = getVector3VertexProperty(property);
        Scalar normalizer=1; /// Originally no change
        foreach(Vertex vit,mesh->vertices()){
            Vector3 stableLaplacian = meanValueLaplacianVector(hweight, vit,true);
            Vector3 unstableLaplacian = meanValueLaplacianVector(hweight, vit,false) / varea[vit];
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
    Vector3 meanValueLaplacianVector(ScalarHalfedgeProperty hweight, Vertex v, bool autonormalize=true){
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
