#pragma once
#include "SurfaceMeshHelper.h"

/// @{ Default property names definition
    #define VGAUSSIANCURV   "v:gausscurv"
    #define EDIHEDRAL       "e:dihedral"    
    #define VANISOTROPY     "v:gausscurv"
    #define VABSMEANCURV    "v:absmeancurv"
/// @}

/// See: Dyn et al. "Optimizing 3D Triangulations Using Discrete Curvature Analysis" 
/// Assumption: manifold triangular mesh
class CurvatureEstimationHelper : public virtual SurfaceMeshHelper{
protected:
    const Scalar PI;                              /// acos(-1)
    ScalarVertexProperty vAbsoluteMeanCurvature;  /// NULL
    ScalarVertexProperty vGaussianCurvature;      /// NULL
    ScalarVertexProperty vAnisotropy;             /// NULL
    ScalarEdgeProperty   eDihedral;               /// NULL
    
public:
    CurvatureEstimationHelper(SurfaceMeshModel* mesh) : SurfaceMeshHelper(mesh), PI(acos(-1.0)){
        varea = mesh->get_vertex_property<Scalar>(VAREA);
        vGaussianCurvature = mesh->get_vertex_property<Scalar>(VGAUSSIANCURV);
        eDihedral = mesh->get_edge_property<Scalar>(EDIHEDRAL);      
        fnormal = mesh->get_face_property<Vector3>(FNORMAL);
    }   

#if 0    
    Surface_mesh::Edge_property<Scalar> computeEdgesDihedralAngle(char* property="e:dihedral"){
        if(!fnormal.is_valid()) throw MissingPropertyException("Requires fnormal");
        edihedral = mesh->edge_property<Scalar>(property);
        foreach(Edge e, mesh->edges()){
            Vector3 f0n = fnormal[ mesh->face( mesh->halfedge(e,0) ) ];
            Vector3 f1n = fnormal[ mesh->face( mesh->halfedge(e,1) ) ];           
            Scalar dotp = qMax(qMin(dot(f0n,f1n),1.0f),-1.0f);
            qDebug() << f0n << f1n << dotp;
            edihedral[e] = acos( dotp );
            Q_ASSERT(!isnan(edihedral[e]));
        }
        return edihedral;
    }
#else
    Surface_mesh::Edge_property<Scalar> computeEdgesDihedralAngle(char* property="e:dihedral"){
        if(!fnormal) throw MissingPropertyException("Requires fnormal");
        
        eDihedral = mesh->edge_property<Scalar>(property);
        foreach(Edge e, mesh->edges()){
            Halfedge h0 = mesh->halfedge(e,0);
            Halfedge h1 = mesh->halfedge(e,1);
            Point w0 = points[mesh->to_vertex(mesh->next_halfedge(h0))]; // Wing vertex of Face 0
            Point w1 = points[mesh->to_vertex(mesh->next_halfedge(h1))]; // Wing vertex of Face 1
            Vector3 f0n = -fnormal[ mesh->face( h0 ) ];
            Vector3 f1n = -fnormal[ mesh->face( h1 ) ];
            Scalar dotp = qMax(qMin(dot(f0n,f1n),1.0),-1.0);
            // qDebug() << dotp << acos(dotp)*180/PI;
            Scalar sign = (dot(w1-w0,f0n)>=0) ? +1:-1;
            eDihedral[e] = sign*acos(dotp);
            // qDebug() << sign << edihedral[e];
            Q_ASSERT(!isnan(eDihedral[e]));
        }
        return eDihedral;
    }
#endif
    
    Surface_mesh::Vertex_property<Scalar> computeVerticesGaussianCurvature(char* property=VGAUSSIANCURV){
        if(!varea) throw MissingPropertyException(VAREA);        
        vGaussianCurvature = mesh->vertex_property<Scalar>(property);
        foreach(Vertex v, mesh->vertices())
            vGaussianCurvature[v] = gaussianCurvatureAt(v);
        return vGaussianCurvature;
    }
    
    Surface_mesh::Vertex_property<Scalar> computeVerticesAbsoluteMeanCurvature(char* property=VABSMEANCURV){
        /// Attempt to retrieve from a previous call or by name       
        if(!eDihedral) throw MissingPropertyException(EDIHEDRAL);
        if(!varea)     throw MissingPropertyException(VAREA);      
        
        vAbsoluteMeanCurvature = mesh->vertex_property<Scalar>(property);
        foreach(Vertex v, mesh->vertices())
            vAbsoluteMeanCurvature[v] = absoluteMeanCurvatureAt(v);
        return vAbsoluteMeanCurvature;
    }
   
    ScalarVertexProperty computeVerticesLocalAnisotropy(char* property="v:anisotropy"){
        if(!vAbsoluteMeanCurvature) throw MissingPropertyException("Absolute Mean Curvature");
        if(!vGaussianCurvature) throw MissingPropertyException("Gaussian Curvature");
        
        vAnisotropy = mesh->vertex_property<Scalar>(property,0);
        foreach(Vertex v, mesh->vertices()){
            Scalar AH = vAbsoluteMeanCurvature[v];
            Scalar G  = vGaussianCurvature[v];
            Q_ASSERT(!isnan(AH));
            Q_ASSERT(!isnan(G));
            Scalar sqrtrmarg=AH*AH-G;
            Scalar sqrtrm = (sqrtrmarg>=0)?sqrt(sqrtrmarg):0.0f;
            Scalar abskmin = fabs(AH-sqrtrm);
            Scalar abskmax = fabs(AH+sqrtrm);
            vAnisotropy[v] = abskmin / abskmax;
            // qDebug() << "AH" << AH << "G" << G;
            // qDebug() << "absmin/absmax = anisotropy" << abskmin << "/"<< abskmax << "=" << anisotropy[v];
        }
        Q_ASSERT(vAnisotropy>=0 && vAnisotropy<=1);
        return vAnisotropy;
    }
    


/// Should not call these directly as they expect data members initialized and perform no checks...
protected:
    Scalar gaussianCurvatureAt(Vertex v){
        Scalar retval = 2*PI;
        Point pcurr = points[v];
        foreach(Halfedge h, mesh->onering_hedges(v)){
            Vector3 vprev = (points[mesh->to_vertex(h)]-pcurr).normalize();
            Vector3 vnext = (points[mesh->to_vertex(mesh->next_halfedge(h))]-pcurr).normalize();
            Scalar dotp = qMax( qMin( dot(vprev,vnext),1.0 ), -1.0 );
            retval -= acos( dotp );
        }
        /// Area normalization
        retval /= varea[v];
        return retval;
    }

    /// careful when calling this directly...
    Scalar absoluteMeanCurvatureAt(Vertex v){
        Scalar meank;
        Point v0 = points[v];
        foreach(Halfedge h, mesh->onering_hedges(v)){
            Edge e = mesh->edge(h);
            Point vi = points[mesh->to_vertex(h)];
            Scalar enorm = (vi-v0).norm();
            meank += enorm*eDihedral[e];
        }
        /// Area normalization
        meank /= 4;
        meank /= varea[v];
        Q_ASSERT(!isnan(meank));
        return meank;
    }
    
};
