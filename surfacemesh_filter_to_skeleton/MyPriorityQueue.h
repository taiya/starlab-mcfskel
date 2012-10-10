#pragma once
#include <set>

#include "CurveskelHelper.h"
namespace CurveskelTypes{

class MyPriorityQueue{
public:
    class EdgeCompareFunctor{
    public:
        ScalarEdgeProperty elengths;
        
        EdgeCompareFunctor(CurveskelModel* skel){
            elengths = skel->edge_property<Scalar>("e:length");
        }
        
        bool operator() (const Edge& lhs, const Edge& rhs) const{
            Scalar p0 = elengths[lhs];
            Scalar p1 = elengths[rhs];
            return (p0 == p1) ? (lhs.idx()<rhs.idx()) : (p0 < p1);
        }
    };   
    EdgeCompareFunctor compareFunctor;
    std::set<Edge, EdgeCompareFunctor> set;
        
    MyPriorityQueue(CurveskelModel* skel) : compareFunctor(skel), set(compareFunctor){}
    
    void insert(Edge edge, Scalar length){
        compareFunctor.elengths[edge] = length;
        set.insert(edge);
    }
    
    bool update(Edge edge, Scalar /*length*/){
        // remove & reinsert
        //qDebug() << "updating: " << edge.idx();

		// erase the edge
        int nerased = set.erase(edge);

        if( nerased != 1 ){
            qDebug() << "I was supposed to erase one element, but I erased: " << nerased; 
			return false;  
        }

		// re-insert it
        set.insert(edge);

		return true;
    }
    
    bool empty(){ 
        return set.empty(); 
    }
    
    Edge pop(){
        Edge e = *(set.begin());
        //qDebug() << "popping: " << e.idx() << "[" << compareFunctor.elengths[e] << "]";
        set.erase(e);
        compareFunctor.elengths[e] = -1.0;
        return e;
    }

	bool has(Edge edge)
	{
		return set.find(edge) != set.end();
	}
};

}
