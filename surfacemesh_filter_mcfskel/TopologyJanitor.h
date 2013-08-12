#pragma once
#include "SurfaceMeshHelper.h"

#ifdef WIN32
#define NAN std::numeric_limits<Scalar>::signaling_NaN()
namespace std{  bool isnan(double x){ return _isnan(x); }
                bool isinf(double x){ return _finite(x); } }
#endif

class TopologyJanitor : public virtual SurfaceMeshHelper{
public:
    TopologyJanitor(SurfaceMeshModel* mesh) : SurfaceMeshHelper(mesh){}
    QString cleanup(Scalar short_edge, Scalar edgelength_TH, Scalar alpha){
        Size nv_prev = mesh->n_vertices();
        Counter numCollapses = iteratively_coolapseShortEdges(edgelength_TH);
        Counter numSplits = iteratively_splitFlatTriangles(short_edge,alpha);
        QString retval;
        retval.sprintf("Topology update: #V %d ==> %d [ #Collapses: %d, #Splits: %d]",nv_prev,mesh->n_vertices(),numCollapses, numSplits);
        return retval;
    }
protected:
    virtual ScalarHalfedgeProperty cacheAngles(Scalar short_edge){
        /// Store halfedge opposite angles
        ScalarHalfedgeProperty halpha = mesh->halfedge_property<Scalar>("h:alpha",0);
        foreach(Face f,mesh->faces()){
            Halfedge h_a = mesh->halfedge(f);
            Halfedge h_b = mesh->next_halfedge(h_a);
            Halfedge h_c = mesh->next_halfedge(h_b);
            Edge e_a = mesh->edge(h_a);
            Edge e_b = mesh->edge(h_b);
            Edge e_c = mesh->edge(h_c);
    
            /// Edge lengths
            Scalar a = mesh->edge_length(e_a), a2=a*a;
            Scalar b = mesh->edge_length(e_b), b2=b*b;
            Scalar c = mesh->edge_length(e_c), c2=c*c;
                
            /// A degenerate triangle will never undergo a split (but rather a collapse...)        
            if( a<short_edge || b<short_edge || c<short_edge ){
                halpha[h_a] = -1;
                halpha[h_b] = -1;
                halpha[h_c] = -1;
            } else {
                /// Opposite angles (from law of cosines)
                halpha[h_a] = acos( qBound(-1.0, (-a2 +b2 +c2)/(2*  b*c), +1.0) );
                halpha[h_b] = acos( qBound(-1.0, (+a2 -b2 +c2)/(2*a  *c), +1.0) );
                halpha[h_c] = acos( qBound(-1.0, (+a2 +b2 -c2)/(2*a*b  ), +1.0) );
            }
            
    #if 0
            if(!(fabs( (halpha[h_a]+halpha[h_b]+halpha[h_c])-3.1415 )<1e-3) ){
                qDebug() << "Detected bad angles: " << halpha[h_a] << halpha[h_b] << halpha[h_c];
                qDebug() << "Edge lengths: " << a << b << c;
                // qDebug() << (+a2 +b2 -c2)/(2*a*b  );
                
                Vector3 p0 = points[ mesh->to_vertex(h_a) ];
                Vector3 p1 = points[ mesh->to_vertex(h_b) ];
                Vector3 p2 = points[ mesh->to_vertex(h_c) ];
                drawArea->drawPoint(p0,2);
                drawArea->drawPoint(p1,2);
                drawArea->drawPoint(p2,2);            
            }
    #endif
        }
        
        return halpha;
    }

    virtual Counter collapser(Scalar short_edge){
        Vector3VertexProperty points = mesh->get_vertex_property<Point>("v:point");
        BoolVertexProperty visfixed = mesh->get_vertex_property<bool>("v:isfixed");
        Counter count=0;
        foreach(Edge e,mesh->edges()){
            Halfedge h = mesh->halfedge(e,0);
            Vertex v0 = mesh->from_vertex(h);
            Vertex v1 = mesh->to_vertex(h);
            /// Don't collapse fixed edges...!!
            if(visfixed[v0] && visfixed[v1]) continue;
            if(mesh->edge_length(e)<short_edge){
                if(!mesh->is_deleted(h) && mesh->is_collapse_ok(h)){
                    points[v1] = (points[v0]+points[v1])/2.0f;
                    mesh->collapse(h);
                    count++;
                }
            }
        }
        return count;
    }    
    virtual Counter splitter(Scalar short_edge, Scalar TH_ALPHA /*110*/){
        Vector3VertexProperty points = mesh->get_vertex_property<Point>("v:point");
        
        /// Keep track / decide which to split    
        TH_ALPHA *= (3.14/180);
    
        /// Compute halpha
        ScalarHalfedgeProperty halpha = cacheAngles(short_edge);        
        
        /// Splitting section
        BoolVertexProperty visfixed = mesh->get_vertex_property<bool>("v:isfixed");
        Scalar numsplits=0;
        BoolVertexProperty vissplit = mesh->vertex_property<bool>("v:issplit",false);
        foreach(Edge e, mesh->edges()){
            Vertex v0 = mesh->vertex(e,0);
            Vertex v1 = mesh->vertex(e,1);
            
            /// Don't collapse fixed!!
            if(visfixed[v0] && visfixed[v1]) continue;
            
            Halfedge h0 = mesh->halfedge(e,0);
            Halfedge h1 = mesh->halfedge(e,1);
    
            /// Should a split take place?
            Scalar alpha_0 = halpha[ h0 ];
            Scalar alpha_1 = halpha[ h1 ];
            if(alpha_0<TH_ALPHA || alpha_1<TH_ALPHA) continue;
            
            /// Which side should I split?
            Vertex w0 = mesh->to_vertex( mesh->next_halfedge(h0) );
            Vertex w1 = mesh->to_vertex( mesh->next_halfedge(h1) );
            Vertex wsplitside = (alpha_0>alpha_1) ? w0 : w1;
            
            /// Project side vertex on edge
            Point p0 = points[mesh->vertex(e,0)];
            Point p1 = points[mesh->vertex(e,1)];
            Vector3 projector = (p1-p0).normalized();
            Vector3 projectee = points[wsplitside]-p0;
            Scalar t = dot(projector, projectee);
            
            Q_ASSERT(!std::isnan(t));
            Vector3 newpos = p0 + t*projector;
            
            /// Perform the split at the desired location
            Vertex vnew = mesh->split(e,newpos);
            vissplit[vnew] = true;
            numsplits++;
        }
        return numsplits;
    }

protected:    
    Counter iteratively_splitFlatTriangles(Scalar short_edge /*1e-10*/, Scalar TH_ALPHA /*110*/){
        Counter new_splits=0;
        Counter tot_splits=0;
        do{
            new_splits = splitter(short_edge,TH_ALPHA);
            tot_splits += new_splits;
            // qDebug() << "new splits: " << new_collapses;
        } while(new_splits>0);
        return tot_splits;
    }
    Counter iteratively_coolapseShortEdges(Scalar edgelength_TH){
        /// This could be done more efficiently, by keeping 
        /// track of possible rings where new collapses took
        /// place... but I am lazy...
        Counter new_collapses=0;
        Counter tot_collapses=0;
        do{
            new_collapses = collapser(edgelength_TH);
            tot_collapses += new_collapses;
            // qDebug() << "new collapses: " << new_collapses;
        } while(new_collapses>0);
        return tot_collapses;
    }
};
