#pragma once

#include <nanoflann.hpp>
#include <vector>

/// @note Vector3 should have:
///    1) &vector[0] point to a value_type[3] 
///    2) a ::value_type
template< class Vector3 >
class NanoKDTree3{
    
/// @{ internal
private:
    /// The scalar type of the system
    typedef typename Vector3::value_type value_type; 
   
    /// Required by nanoflann to store data. 
    /// @internal Technically the "build" phase should then just provide a Random 
    /// Access Iterator.. removing the need altogether to copy the data!!!
    struct PointCloud{
        std::vector<Vector3>  pts;
    
        inline size_t kdtree_get_point_count() const { return pts.size(); }
    
        inline value_type kdtree_distance(const value_type* p1, const size_t idx_p2, size_t size) const{
            const value_type d0=p1[0]-pts[idx_p2].x();
            const value_type d1=p1[1]-pts[idx_p2].y();
            const value_type d2=p1[2]-pts[idx_p2].z();
            size = size; /// @todo Ibraheem ????
            return sqrt(d0*d0 + d1*d1 + d2*d2);
        }
    
        inline value_type kdtree_get_pt(const size_t idx, int dim) const{
            if (dim==0) return pts[idx].x();
            else if (dim==1) return pts[idx].y();
            else return pts[idx].z();
        }
    
        template <class BBOX>
        bool kdtree_get_bbox(BBOX &) const { return false; }
    } cloud;
    
    /// Internal specification of nanoflann's kdtree
    typedef nanoflann::KDTreeSingleIndexAdaptor< nanoflann::L2_Simple_Adaptor<value_type, PointCloud >, PointCloud, /*dim*/ 3 > my_kd_tree;
    
    /// Pointer to tree instance
    my_kd_tree * tree;
/// @}
      
public:

/// @{ constructor/destructor  
    NanoKDTree3(){ tree = NULL; }
    ~NanoKDTree3(){ if(tree) delete tree; }

    template< class RandomAccessIterator >
    NanoKDTree3(RandomAccessIterator begin, RandomAccessIterator beyond){
        tree = NULL;
        build(begin, beyond);        
    }
    
    template< class ForwardIterator >
    void build(ForwardIterator begin, ForwardIterator beyond){
        cloud.pts.clear();
        for(ForwardIterator it=begin; it!=beyond; ++it)
            cloud.pts.push_back( Vector3((*it).x(),(*it).y(),(*it).z()) );
        this->build();
    }

    /// To be used with insert    
    void build(){
        if(tree) delete tree;
        tree = new my_kd_tree(3 /*dim*/, cloud, nanoflann::KDTreeSingleIndexAdaptorParams(10 /* max leaf */) );
		tree->buildIndex();        
    }
    
    /// Static insert, tree deleted otherwise
    void insert(Vector3 point){
        if(tree) delete tree;
        tree = NULL;
        cloud.pts.push_back(point);
    }
/// @}
       
/// @{ queries return types
    typedef std::pair<size_t, value_type> KDResultPair;
    typedef std::vector< KDResultPair > KDResults;
/// @} 
   
/// @{ queries @todo: why I cannot pass "const Vector3&" for the query?
    /// This one return a custom structure
    size_t k_closest(double query[3], int k, KDResults& ret_matches){
        assert(tree);
        
        k = k < (int)cloud.pts.size() ? k : cloud.pts.size();

		ret_matches.clear();
		ret_matches.resize(k);

		std::vector<size_t> ret_index(k);
		std::vector<double> out_dist(k);

        tree->knnSearch(query, k, &(ret_index[0]), &(out_dist[0]));

		for(int i = 0; i < k; i++)
			ret_matches[i] = std::make_pair(ret_index[i], out_dist[i]);

		return k;
	}
       
    /// @todo: this one returns a type-free result (only uses std::)
    void k_closest(double query[3], int k, std::vector<size_t>& /*=*/ idxs, std::vector<value_type>& /*=*/ dists){
        assert(tree);
        
        idxs.clear();
        idxs.resize(k,0);
        dists.clear();
        dists.resize(k,value_type(0));
        tree->knnSearch(query, k, &idxs[0], &dists[0]);              
    }
    
    size_t ball_search(double query[3], double search_radius){
        assert(tree);
        
        KDResults ret_matches;
        return ball_search(query, search_radius, ret_matches);
    }

    size_t ball_search(double query[3], double search_radius, KDResults & ret_matches){
        assert(tree);

        ret_matches.clear();
		nanoflann::SearchParams params;
		//params.sorted = false;
		return tree->radiusSearch(query, search_radius, ret_matches, params);
	}

    int closest_index(double query[3]){
        assert(tree);
    
        KDResults match;
        this->k_closest(query, 1, match);

        if(!match.size())
            return -1;
        else
            return match[0].first;
    }
    
    Vector3& closest(double query[3]){
        assert(tree);

        KDResults match;
        this->k_closest(query, 1, match);
        assert(match.size()==1);
        assert(match[0].first<cloud.pts.size());
        return cloud.pts[ match[0].first ] ;
    }

	inline bool has(double query[3], Scalar eps_distance = 1e-7){
        assert(tree);

        KDResults match;
		ball_search(query, eps_distance, match);
		return match.size();
	}
/// @}
    
/// @{
public:
    template< class Point3 >
    void k_closest(Point3 query, int k, std::vector<size_t>& /*=*/ idxs, std::vector<value_type>& /*=*/ dists){
        double _query[3] = {query.x(), query.y(), query.z()};
        k_closest(_query, k, idxs, dists);
    }
    
    template< class Point3 >
    std::vector<size_t>  k_closest(Point3 query, int k){
        assert(k>0);
        std::vector<size_t> retval;
        std::vector<double> dists;
        double _query[3] = {query.x(), query.y(), query.z()};
        k_closest(_query, k, retval, dists);
        return retval;
    }
    
    template< class Point3 >
    int closest_index(Point3 query){
        double _query[3] = {query.x(), query.y(), query.z()};
        return closest_index(_query);
    }
    
    template< class Point3 >
    std::vector<size_t> ball_search(Point3 query, double search_radius){
        std::vector<size_t> retval;
        double _query[3] = {query.x(), query.y(), query.z()};
        KDResults ret_matches;
        ball_search(_query, search_radius, ret_matches);
        for(size_t i=0; i<ret_matches.size(); i++)
            retval.push_back(ret_matches[i].first);
        return retval;
    }
/// @}
};
