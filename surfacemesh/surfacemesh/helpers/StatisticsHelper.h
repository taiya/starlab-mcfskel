#pragma once
#include <string>
#include <float.h>
#include "SurfaceMeshHelper.h"

#ifdef WIN32
#define NAN std::numeric_limits<Scalar>::signaling_NaN()
#endif

namespace SurfaceMesh{

struct MeanHelper{
private:
    Scalar sum;
    unsigned int counter;
public:
    MeanHelper() : sum(0), counter(0){}
    void push(Scalar value){
        sum+=value;
        counter++;
    }
    Scalar value(){
        return sum/counter;
    }
    operator Scalar(){
        return value();   
    }
};

struct VarianceHelper{
private:
    Scalar mean;
    Scalar variance;
    unsigned int counter;
public:
    VarianceHelper(Scalar mean) : mean(mean),variance(0),counter(0){}

    void push(Scalar value){
        variance += (value-mean)*(value-mean);
        counter ++;
    }
    Scalar value(){
        return variance/( (Scalar)(counter-1) );
    }
    operator Scalar(){
        return value();   
    }
};
 

template <class TYPE=Scalar>
class StatisticsHelper : public virtual SurfaceMeshHelper{
public:
    StatisticsHelper(SurfaceMeshModel* mesh) : SurfaceMeshHelper(mesh){}
        
    Scalar mean(const std::string property){
        Scalar min=NAN,max=NAN,mean=NAN,var=NAN;

        /// Use first letter to pick and retrieve property
        if(property[0]=='v') statistics( mesh->get_vertex_property<TYPE>   (property),min,max,mean,var );
        if(property[0]=='e') statistics( mesh->get_edge_property<TYPE>     (property),min,max,mean,var );
        if(property[0]=='h') statistics( mesh->get_halfedge_property<TYPE> (property),min,max,mean,var );
        
        return mean;
    }
   
    QString statistics(const std::string property){
        Scalar min=NAN,max=NAN,mean=NAN,var=NAN;
        
        /// Use first letter to pick and retrieve property
        if(property[0]=='v') statistics( mesh->get_vertex_property<TYPE>   (property),min,max,mean,var );
        if(property[0]=='e') statistics( mesh->get_edge_property<TYPE>     (property),min,max,mean,var );
        if(property[0]=='h') statistics( mesh->get_halfedge_property<TYPE> (property),min,max,mean,var );
       
        /// The assemble the string
        QString retval;
        QTextStream sout(&retval);
        sout << "[" << property.c_str() << "]: " << mean << " +/- " << sqrt(var) << " [" << min << " " << max << "], ";
        return retval;
    }
    
    
    void statistics(Surface_mesh::Vertex_property<TYPE> p, Scalar& min, Scalar& max, Scalar& mean, Scalar& var){
        MeanHelper mh;
        min = +FLT_MAX;
        max = -FLT_MAX;
        foreach(Vertex v, mesh->vertices()){
            min = qMin(min,p[v]); 
            max = qMax(max,p[v]); 
            mh.push(p[v]);
        }
        mean = mh.value();
        VarianceHelper vh(mh);
        foreach(Vertex v, mesh->vertices())
            vh.push(p[v]);
        var = vh.value();  
    }
    void statistics(Surface_mesh::Halfedge_property<TYPE> p, Scalar& min, Scalar& max, Scalar& mean, Scalar& var){
        MeanHelper mh;
        min = +FLT_MAX;
        max = -FLT_MAX;
        
        foreach(Edge e, mesh->edges()){
            Halfedge h0 = mesh->halfedge(e,0);
            Halfedge h1 = mesh->halfedge(e,1);
            min = qMin(min,p[h0]); 
            max = qMax(max,p[h0]); 
            min = qMin(min,p[h1]); 
            max = qMax(max,p[h1]); 

            mh.push(p[h0]);            
            mh.push(p[h1]);            
        }
        mean = mh.value();
        VarianceHelper vh(mh);
        foreach(Edge e, mesh->edges()){
            Halfedge h0 = mesh->halfedge(e,0);
            Halfedge h1 = mesh->halfedge(e,1);
            vh.push(p[h0]);
            vh.push(p[h1]);
        }
        var = vh.value();
    }
    void statistics(Surface_mesh::Edge_property<TYPE> p, Scalar& min, Scalar& max, Scalar& mean, Scalar& var){
        MeanHelper mh;
        min = +FLT_MAX;
        max = -FLT_MAX;
        
        foreach(Edge e, mesh->edges()){
            min = qMin(min,p[e]); 
            max = qMax(max,p[e]); 
            mh.push(p[e]);            
            mh.push(p[e]);            
        }
        mean = mh.value();
        VarianceHelper vh(mh);
        foreach(Edge e, mesh->edges()){
            vh.push(p[e]);
            vh.push(p[e]);
        }
        var = vh.value();
    }
    
};

typedef StatisticsHelper<Scalar> ScalarStatisticsHelper;

} /// namespace
