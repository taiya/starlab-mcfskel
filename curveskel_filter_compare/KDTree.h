#pragma once
#include <vector>    // point datatype
#include <math.h>    // fabs operation
#include "MyHeaps.h" // priority queues
#include "float.h"   // max floating point number

#ifdef MATLAB
#include "mex.h"
#endif

using namespace std;

typedef vector<double> KDPoint;

/// The root node is stored in position 0 of nodesPtrs
#define ROOT 0

/// L2 distance (in dimension ndim) between two points
inline double distance_squared( const vector<double>& a, const vector<double>& b){
    double d = 0;
    double N = a.size();
    for( int i=0; i<N; i++ )
        d += (a[i]-b[i])*(a[i]-b[i]);
    return d;
}

struct Node{
    double key;	///< the key (value along k-th dimension) of the split
    int LIdx;	///< the index to the left sub-tree (-1 if none)
	int	RIdx;	///< the index to the right sub-tree (-1 if none)
	int	pIdx;   ///< index of stored data-point (NOTE: only if isLeaf)

    Node(){ LIdx=RIdx=key=pIdx=-1; }
    inline bool isLeaf() const{ return pIdx>=0; }
};

class KDTree{
    /// @{ kdtree constructor/destructor
    public:
        KDTree(){}                           ///< Default constructor (only for load/save)
        KDTree(const vector<KDPoint>& points); ///< tree constructor
        ~KDTree();                           ///< tree destructor
    private:
        int build_recursively(vector< vector<int> >& sortidx, vector<char> &sidehelper, int dim);       
        // int heapsort(int dim, vector<int>& idx, int len);
    /// @}
        
    /// @{ basic info
    public:
        inline int size(){ return points.size(); } ///< the number of points in the kd-tree
        inline int ndims(){ return ndim; } ///< the number of dimensions of a point in the kd-tree
    /// @}
        
    /// @{ core kdtree data
    private:
        int ndim;                 ///< Number of dimensions of the data (>0)
        int npoints;              ///< Number of stored points
        vector<KDPoint> points;     ///< Points data, size ?x?
        vector<Node*> nodesPtrs;  ///< Tree node pointers, size ?x?
    /// @}
   
    /// @{ Debuggging helpers
	public:
        void linear_tree_print() const; 
        void left_depth_first_print( int nodeIdx=0 ) const;
        void print_tree( int index=0, int level=0 ) const;
        void leaves_of_node( int nodeIdx, vector<int>& indexes );
    /// @}
       
    /// @{ Knn Search & helpers
    public:
        int closest_point(const KDPoint& p);
        void closest_point(const KDPoint &p, int &idx, double &dist);
        void k_closest_points(const KDPoint& Xq, int k, vector<int>& idxs, vector<double>& distances);
    private:
        void knn_search( const KDPoint& Xq, int nodeIdx = 0, int dim = 0);
        bool ball_within_bounds(const KDPoint& Xq);
        double bounds_overlap_ball(const KDPoint& Xq);
    private:
        int k;					  ///< number of records to search for
        KDPoint Bmin;  		 	  ///< bounding box lower bound
        KDPoint Bmax;  		      ///< bounding box upper bound
        MaxHeap<double> pq;  	  ///< <key,idx> = <distance, node idx>
        bool terminate_search;    ///< true if k points have been found
    /// @}        

	/// @{ Points in hypersphere (ball) query
    public: 
        void ball_query( const KDPoint& point, const double radius, vector<int>& idxsInRange, vector<double>& distances );
    private: 
        void ball_bbox_query(int nodeIdx, KDPoint& pmin, KDPoint& pmax, vector<int>& inrange_idxs, vector<double>& distances, const KDPoint& point, const double& radiusSquared, int dim=0);
    /// @}       
    
    /// @{ Range (box) query 
    public: 
        void range_query( const KDPoint& pmin, const KDPoint& pmax, vector<int>& inrange_idxs, int nodeIdx=0, int dim=0 );
    private:
        bool lies_in_range( const KDPoint& p, const KDPoint& pMin, const KDPoint& pMax );
    /// @}
};

//----------------------------------------------------------------------------------------
//                                  
//                                  Implementation
//                                  
//----------------------------------------------------------------------------------------

/**
 * Creates a KDtree filled with the provided data.
 *
 * @param points   a vector< vector<double> > containing the point data
 * 				   the number of points and the dimensionality is inferred
 *                 by the data
 */
KDTree::KDTree(const vector<KDPoint>& points){
    // initialize data
    this -> npoints   = points.size();
    this -> ndim      = points[0].size();
    this -> points    = points;
    nodesPtrs.reserve( npoints );
    
    // used for sort-based tree construction
    // tells whether a point should go to the left or right 
    // array in the partitioning of the sorting array
    vector<char> sidehelper(npoints,'x');
    
    // Invoke heap sort generating indexing vectors
    // sorter[dim][i]: in dimension dim, which is the i-th smallest point?
    vector< MinHeap<double> > heaps(ndim, npoints);
    for( int dIdx=0; dIdx<ndim; dIdx++ )
        for( int pIdx=0; pIdx<npoints; pIdx++ )
            heaps[dIdx].push( points[pIdx][dIdx], pIdx );
    vector< vector<int> > sorter( ndim, vector<int>(npoints,0) );
    for( int dIdx=0; dIdx<ndim; dIdx++ )
        heaps[dIdx].heapsort( sorter[dIdx] );

    build_recursively(sorter, sidehelper, 0);
}

KDTree::~KDTree(){
    for (unsigned int i=0; i < nodesPtrs.size(); i++)
        delete nodesPtrs[i];
}    


/**
 * Algorithm that recursively performs median splits along dimension "dim"
 * using the pre-prepared information given by the sorting.
 *
 * @param sortidx: the back indexes produced by sorting along every dimension used for median computation
 * @param pidx:    a vector of indexes to active elements
 * @param dim:     the current split dimension
 *
 * @note this is the memory-friendly version
 */
void print_sorter(const char* message, vector< vector<int> >& srtidx){
    cout << message << endl;
    for (unsigned int j=0; j <srtidx.size(); j++){
        for (unsigned int i=0; i < srtidx[j].size(); i++)
            cout << srtidx[j][i] << " ";
        cout << endl;
    }    
}
int KDTree::build_recursively(vector< vector<int> >& sorter, vector<char>& sidehelper, int dim){
    // Current number of elements
    int numel = sorter[dim].size();
    
    // Stop condition
    if(numel == 1) {
        Node *node = new Node();		// create a new node
        int nodeIdx = nodesPtrs.size(); // its address is
        nodesPtrs.push_back( node ); 	// important to push back here
        node->LIdx = -1;				// no child
        node->RIdx = -1;    			// no child
        /// @todo take it from sorter
        node->pIdx = sorter[dim][0];    // the only index available
        node->key = 0;					// key is useless here
        return nodeIdx;
    }
    
    // defines median offset
    // NOTE: pivot goes to the LEFT sub-array
    int iMedian = floor((numel-1)/2.0);
    int pidxMedian = sorter[dim][iMedian];
    int nL = iMedian+1;
    int nR = numel-nL;
    
    // Assign l/r sides
    for(int i=0; i<(int)sorter[dim].size(); i++){
        int pidx = sorter[dim][i];
        sidehelper[ pidx ] = (i<=iMedian) ? 'l':'r';
    }
    
    // allocate the vectors initially with invalid data
    vector< vector<int> > Lsorter(ndim, vector<int>(nL,-1));
    vector< vector<int> > Rsorter(ndim, vector<int>(nR,-1));

    for(int idim=0; idim<ndims(); idim++){
        int iL=0, iR=0;
        for(int i=0; i<(int)sorter[idim].size(); i++){
            int pidx = sorter[idim][i];
            if(sidehelper[pidx]=='l')
                Lsorter[idim][iL++] = pidx;
            if(sidehelper[pidx]=='r')
                Rsorter[idim][iR++] = pidx;
        }
    }
    
#if DEBUG
    if(numel>2){
        cout << "---- SPLITTING along " << dim << endl;
        print_sorter("original: ", sorter);
        print_sorter("L: ", Lsorter);
        print_sorter("R: ", Rsorter);
    }
#endif
    
    // CREATE THE NODE
    Node* node = new Node();
    int nodeIdx = nodesPtrs.size(); //size() is the index of last element+1!!
    nodesPtrs.push_back( node ); //important to push back here
    node->pIdx  	= -1; //not a leaf
    node->key  		= points[ pidxMedian ][ dim ];
    node->LIdx 		= build_recursively( Lsorter, sidehelper, (dim+1)%ndim );
    node->RIdx 		= build_recursively( Rsorter, sidehelper, (dim+1)%ndim );
    return nodeIdx;
}

/**
 * Prints the tree traversing linearly the structure of nodes
 * in which the tree is stored.
 */
void KDTree::linear_tree_print() const{
    for (unsigned int i=0; i < nodesPtrs.size(); i++) {
#ifdef MATLAB
        Node* n = nodesPtrs[i];
        if(n==NULL) mexErrMsgTxt("%d-th node is NULL.");
        if(n->isLeaf())
            mexPrintf("Node[%d] P[%d]\n",i,n->pIdx);
        else
            mexPrintf("Node[%d] key %.2f Children[%d %d]\n",i,n->key,n->LIdx,n->RIdx);
#endif
    }
}

/**
 * Prints the tree in depth first order, visiting
 * the node to the left, then the root, then the node
 * to the right recursively.
 *
 * @param nodeIdx the node of the index from which to start printing
 *        (default is the root)
 */
void KDTree::left_depth_first_print( int nodeIdx /*=0*/) const{
    Node* currnode = nodesPtrs[nodeIdx];

    if( currnode -> LIdx != -1 )
        left_depth_first_print( currnode -> LIdx );
    cout << currnode -> key << " ";
    if( currnode -> RIdx != -1 )
        left_depth_first_print( currnode -> RIdx );
}

/**
 * Prints the tree in a structured way trying to make clear
 * the underlying hierarchical structure using indentation.
 *
 * @param index the index of the node from which to start printing
 * @param level the key-dimension of the node from which to start printing
 */
void KDTree::print_tree( int index/*=0*/, int level/*=0*/ ) const{
    Node* currnode = nodesPtrs[index];

    // leaf
    if( currnode->pIdx >= 0 ){
        cout << "--- "<< currnode->pIdx+1 << " --- "; //node is given in matlab indexes
        for( int i=0; i<ndim; i++ ) cout << points[ currnode->pIdx ][ i ] << " ";
        cout << endl;
    }
    else
        cout << "l(" << level%ndim << ") - " << currnode->key << " nIdx: " << index << endl;

    // navigate the childs
    if( currnode -> LIdx != -1 ){
        for( int i=0; i<level; i++ ) cout << "  ";
        cout << "left: ";
        print_tree( currnode->LIdx, level+1 );
    }
    if( currnode -> RIdx != -1 ){
        for( int i=0; i<level; i++ ) cout << "  ";
        cout << "right: ";
        print_tree( currnode->RIdx, level+1 );
    }
}

/**
 * k-NN query: computes the k closest points in the database to a given point
 * and returns their indexes.
 *
 * @param Xq            the query point
 * @param k             the number of neighbors to search for
 * @param idxs          the search results 
 * @param distances     the distances from the points
 *
 */
void KDTree::k_closest_points(const KDPoint& Xq, int k, vector<int>& idxs, vector<double>& distances){
    // initialize search data
    Bmin = vector<double>(ndim,-DBL_MAX);
    Bmax = vector<double>(ndim,+DBL_MAX);
    this->k = k;
    this->terminate_search = false;

    // call search on the root [0] fill the queue
    // with elements from the search
    knn_search( Xq );

    // scan the created pq and extract the first "k" elements
    // pop the remaining
    int N = pq.size();
    for (int i=0; i < N; i++) {
        pair<double, int> topel = pq.top();
        pq.pop();
        if( i>=N-k ){
            idxs.push_back( topel.second );
            distances.push_back( sqrt(topel.first) ); // it was distance squared
        }
    }

    // invert the vector, passing first closest results
    std::reverse( idxs.begin(), idxs.end() );
    std::reverse( distances.begin(), distances.end() );
}

/**
 * The algorithm that computes kNN on a k-d tree as specified by the
 * referenced paper.
 *
 * @param nodeIdx the node from which to start searching (default root)
 * @param Xq the query point
 * @param dim the dimension of the current node (default 0, the first)
 *
 * @note: this function and its subfunctions make use of shared
 *        data declared within the data structure: Bmin, Bmax, pq
 *
 * @article{friedman1977knn,
 *          author = {Jerome H. Freidman and Jon Louis Bentley and Raphael Ari Finkel},
 *          title = {An Algorithm for Finding Best Matches in Logarithmic Expected Time},
 *          journal = {ACM Trans. Math. Softw.},
 *          volume = {3},
 *          number = {3},
 *          year = {1977},
 *          issn = {0098-3500},
 *          pages = {209--226},
 *          doi = {http://doi.acm.org/10.1145/355744.355745},
 *          publisher = {ACM},
 *          address = {New York, NY, USA}}
 */
void KDTree::knn_search( const KDPoint& Xq, int nodeIdx/*=0*/, int dim/*=0*/){
    // cout << "at node: " << nodeIdx << endl;
    Node* node = nodesPtrs[ nodeIdx ];
    double temp;

    // We are in LEAF
    if( node -> isLeaf() ){
        double distance = distance_squared( Xq, points[ node->pIdx ] );

        // pqsize is at maximum size k, if overflow and current record is closer
        // pop further and insert the new one
        if( pq.size()==k && pq.top().first>distance ){
            pq.pop(); // remove farther record
            pq.push( distance, node->pIdx ); //push new one
        }
        else if( pq.size()<k )
            pq.push( distance, node->pIdx );

        return;
    }

    ////// Explore the sons //////
    // recurse on closer son
    if( Xq[dim] <= node->key ){
        temp = Bmax[dim]; Bmax[dim] = node->key;
        knn_search( Xq, node->LIdx, (dim+1)%ndim );
        Bmax[dim] = temp;
    }
    else{
        temp = Bmin[dim]; Bmin[dim] = node->key;
        knn_search( Xq, node->RIdx, (dim+1)%ndim );
        Bmin[dim] = temp;
    }
    // recurse on farther son
    if( Xq[dim] <= node->key ){
        temp = Bmin[dim]; Bmin[dim] = node->key;
        if( bounds_overlap_ball(Xq) )
            knn_search( Xq, node->RIdx, (dim+1)%ndim );
        Bmin[dim] = temp;
    }
    else{
        temp = Bmax[dim]; Bmax[dim] = node->key;
        if( bounds_overlap_ball(Xq) )
            knn_search( Xq, node->LIdx, (dim+1)%ndim );
        Bmax[dim] = temp;
    }
}

void KDTree::leaves_of_node( int nodeIdx, vector<int>& indexes ){
    Node* node = nodesPtrs[ nodeIdx ];
    if( node->isLeaf() ){
        indexes.push_back( node->pIdx );
        return;
    }

    leaves_of_node( node->LIdx, indexes );
    leaves_of_node( node->RIdx, indexes );
}

void KDTree::closest_point(const KDPoint &p, int& idx, double& dist){
    vector<int> idxs;
    vector<double> dsts;
    k_closest_points(p,1,idxs,dsts);
    idx = idxs[0];
    dist = dsts[0];
    return;
}

int KDTree::closest_point(const KDPoint &p){
    int idx;
    double dist;
    closest_point(p,idx,dist);
    return idx;
}

/** @see knn_search
 * this function was in the original paper implementation.
 * Was this function useful? How to implement the "done"
 * as opposed to "return" was a mistery. It was used to
 * interrupt search. It might be worth to check its purpose.
 *
 * Verifies if the ball centered in the query point, which
 * radius is the distace from the sample Xq to the k-th best
 * found point, doesn't touches the boundaries of the current
 * BBox.
 *
 * @param Xq the query point
 * @return true if the search can be safely terminated, false otherwise
 */
bool KDTree::ball_within_bounds(const KDPoint& Xq){

    //extract best distance from queue top
    double best_dist = sqrt( pq.top().first );
    // check if ball is completely within BBOX
    for (int d=0; d < ndim; d++)
        if( fabs(Xq[d]-Bmin[d]) < best_dist || fabs(Xq[d]-Bmax[d]) < best_dist )
            return false;
    return true;
}
/** @see knn_search
 *
 * This is the search bounding condition. It checks wheter the ball centered
 * in the sample point, with radius given by the k-th closest point to the query
 * (if k-th closest not defined is \inf), touches the bounding box defined for
 * the current node (Bmin Bmax globals).
 *
 */
double KDTree::bounds_overlap_ball(const KDPoint& Xq){
    // k-closest still not found. termination test unavailable
    if( pq.size()<k )
        return true;

    double sum = 0;
    //extract best distance from queue top
    double best_dist_sq = pq.top().first;
    // cout << "current best dist: " << best_dist_sq << endl;
    for (int d=0; d < ndim; d++) {
        // lower than low boundary
        if( Xq[d] < Bmin[d] ){
            sum += ( Xq[d]-Bmin[d] )*( Xq[d]-Bmin[d] );
            if( sum > best_dist_sq )
                return false;
        }
        else if( Xq[d] > Bmax[d] ){
            sum += ( Xq[d]-Bmax[d] )*( Xq[d]-Bmax[d] );
            if( sum > best_dist_sq )
                return false;
        }
        // else it's in range, thus distance 0
    }

    return true;
}


/**
 * Query all points at distance less or than radius from point
 *
 * @param point the center of the ndim dimensional query ball
 * @param radius the radius of the ndim dimensional query ball
 * @param idxsInRange (return) a collection of indexes of points that fall within
 *        the given ball.
 * @param distances the distances from the query point to the points within the ball
 *
 * @note This is a fairly unefficient implementation for two reasons:
 *       1) the range query is not implemented in its most efficient way
 *       2) all the points in between the bbox and the ball are visited as well, then rejected
 */
void KDTree::ball_query( const KDPoint& point, const double radius, vector<int>& idxsInRange, vector<double>& distances ){
    // create pmin pmax that bound the sphere
    KDPoint pmin(ndim,0);
    KDPoint pmax(ndim,0);
    for (int dim=0; dim < ndim; dim++) {
        pmin[dim] = point[dim]-radius;
        pmax[dim] = point[dim]+radius;
    }
    // start from root at zero-th dimension
    ball_bbox_query( ROOT, pmin, pmax, idxsInRange, distances, point, radius*radius, 0 );
}
/** @see ball_query, range_query
 *
 * Returns all the points withing the ball bounding box and their distances
 *
 * @note this is similar to "range_query" i just replaced "lies_in_range" with "euclidean_distance"
 */
void KDTree::ball_bbox_query(int nodeIdx, KDPoint& pmin, KDPoint& pmax, vector<int>& inrange_idxs, vector<double>& distances, const KDPoint& point, const double& radiusSquared, int dim/*=0*/){
    Node* node = nodesPtrs[nodeIdx];

    // if it's a leaf and it lies in R
    if( node->isLeaf() ){
        double distance = distance_squared(points[node->pIdx], point);
        if( distance <= radiusSquared ){
            inrange_idxs.push_back( node->pIdx );
            distances.push_back( sqrt(distance) );
            return;
        }
    }
    else{
        if(node->key >= pmin[dim] && node->LIdx != -1 )
            ball_bbox_query( node->LIdx, pmin, pmax, inrange_idxs, distances, point, radiusSquared, (dim+1)%ndim);
        if(node->key <= pmax[dim] && node->RIdx != -1 )
            ball_bbox_query( node->RIdx, pmin, pmax, inrange_idxs, distances, point, radiusSquared, (dim+1)%ndim);
    }
}

/**
 * k-dimensional Range query: given a bounding box in ndim dimensions specified by the parameters
 * returns all the indexes of points within the bounding box.
 *
 * @param pmin the lower corner of the bounding box
 * @param pmax the upper corner of the bounding box
 * @param inrange_idxs the indexes which satisfied the query, falling in the bounding box area
 *
 */
void KDTree::range_query( const KDPoint& pmin, const KDPoint& pmax, vector<int>& inrange_idxs, int nodeIdx/*=0*/, int dim/*=0*/ ){
    Node* node = nodesPtrs[nodeIdx];
    //cout << "I am in: "<< nodeIdx << "which is is leaf?" << node->isLeaf() << endl;

    // if it's a leaf and it lies in R
    if( node->isLeaf() ){
        if( lies_in_range(points[node->pIdx], pmin, pmax) ){
            inrange_idxs.push_back( node->pIdx );
            return;
        }
    }
    else{
        if(node->key >= pmin[dim] && node->LIdx != -1 )
            range_query( pmin, pmax, inrange_idxs, node->LIdx, (dim+1)%ndim);
        if(node->key <= pmax[dim] && node->RIdx != -1 )
            range_query( pmin, pmax, inrange_idxs, node->RIdx, (dim+1)%ndim);
    }
}
/** @see range_query
 * Checks if a point lies in the bounding box (defined by pMin and pMax)
 *
 * @param p the point to be checked for
 * @param pMin the lower corner of the bounding box
 * @param pMax the upper corner of the bounding box
 *
 * @return true if the point lies in the box, false otherwise
 */
bool KDTree::lies_in_range( const KDPoint& p, const KDPoint& pMin, const KDPoint& pMax ){
    for (int dim=0; dim < ndim; dim++)
        if( p[dim]<pMin[dim] || p[dim]>pMax[dim] )
            return false;
    return true;
}
