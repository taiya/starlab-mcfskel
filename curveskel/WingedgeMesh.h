#pragma once
#include <exception>
#include <cassert>
#include <vector>
#include <set>
#include <map>
#include <iostream>
#include "StarlabException.h"

namespace CurveskelTypes{

class Wingedge_Base_property_array
{
public:

    /// Default constructor
    Wingedge_Base_property_array(const std::string& name) : name_(name) {}

    /// Destructor.
    virtual ~Wingedge_Base_property_array() {}

    /// Reserve memory for n elements.
    virtual void reserve(size_t n) = 0;

    /// Resize storage to hold n elements.
    virtual void resize(size_t n) = 0;

    /// Free unused memory.
    virtual void free_memory() = 0;

    /// Extend the number of elements by one.
    virtual void push_back() = 0;

    /// Let two elements swap their storage place.
    virtual void swap(size_t i0, size_t i1) = 0;

    /// Return a deep copy of self.
    virtual Wingedge_Base_property_array* clone () const = 0;

    /// Return the name of the property
    const std::string& name() const { return name_; }


protected:

    std::string name_;
};



//== CLASS DEFINITION =========================================================


template <class T>
class WingedgeProperty_array : public Wingedge_Base_property_array
{
public:

    typedef T                                       value_type;
    typedef std::vector<value_type>                 vector_type;
    typedef typename vector_type::reference         reference;
    typedef typename vector_type::const_reference   const_reference;

    WingedgeProperty_array(const std::string& name, T t=T()) : Wingedge_Base_property_array(name), value_(t) {}


public: // virtual interface of Base_property_array

    virtual void reserve(size_t n)
    {
        data_.reserve(n);
    }

    virtual void resize(size_t n)
    {
        data_.resize(n, value_);
    }

    virtual void push_back()
    {
        data_.push_back(value_);
    }

    virtual void free_memory()
    {
        vector_type(data_).swap(data_);
    }

    virtual void swap(size_t i0, size_t i1)
    {
        T d(data_[i0]);
        data_[i0]=data_[i1];
        data_[i1]=d;
    }

    virtual Wingedge_Base_property_array* clone() const
    {
        WingedgeProperty_array<T>* p = new WingedgeProperty_array<T>(name_, value_);
        p->data_ = data_;
        return p;
    }


public:

    /// Get pointer to array (does not work for T==bool)
    const T* data() const
    {
        return &data_[0];
    }

    /// Access the i'th element. No range check is performed!
    reference operator[](int _idx)
    {
        assert( size_t(_idx) < data_.size() );
        return data_[_idx];
    }

    /// Const access to the i'th element. No range check is performed!
    const_reference operator[](int _idx) const
    {
        assert( size_t(_idx) < data_.size());
        return data_[_idx];
    }


private:
    vector_type data_;
    value_type  value_;
};


// specialization for bool properties
template <>
inline const bool*
WingedgeProperty_array<bool>::data() const
{
    assert(false);
    return NULL;
}



//== CLASS DEFINITION =========================================================


template <class T>
class WingedgeProperty
{
public:

    typedef typename WingedgeProperty_array<T>::reference reference;
    typedef typename WingedgeProperty_array<T>::const_reference const_reference;

    friend class Property_container;
    friend class Surface_mesh;


public:

    WingedgeProperty(WingedgeProperty_array<T>* p=NULL) : parray_(p) {}

    void reset()
    {
        parray_ = NULL;
    }

    operator bool() const
    {
        return parray_ != NULL;
    }
    
    bool is_valid() const{
        return parray_ != NULL;
    }

    reference operator[](int i)
    {
        assert(parray_ != NULL);
        return (*parray_)[i];
    }

    const_reference operator[](int i) const
    {
        assert(parray_ != NULL);
        return (*parray_)[i];
    }

    const T* data() const
    {
        assert(parray_ != NULL);
        return parray_->data();
    }


private:

    WingedgeProperty_array<T>& array()
    {
        assert(parray_ != NULL);
        return *parray_;
    }

    const WingedgeProperty_array<T>& array() const
    {
        assert(parray_ != NULL);
        return *parray_;
    }


private:
    WingedgeProperty_array<T>* parray_;
};



//== CLASS DEFINITION =========================================================


class Property_container
{
public:

    // default constructor
    Property_container() : size_(0) {}

    // destructor (deletes all property arrays)
    virtual ~Property_container() { clear(); }

    // copy constructor: performs deep copy of property arrays
    Property_container(const Property_container& _rhs) { operator=(_rhs); }
    
    // assignment: performs deep copy of property arrays
    Property_container& operator=(const Property_container& _rhs)
    {
        if (this != &_rhs)
        {
            clear();
            parrays_.resize(_rhs.n_properties());
            size_ = _rhs.size();
            for (unsigned int i=0; i<parrays_.size(); ++i)
                parrays_[i] = _rhs.parrays_[i]->clone();
        }
        return *this;
    }

    // shallow copier
    void shallow_copy(const Property_container& _rhs){
        if (this != &_rhs)
        {
            clear();
            parrays_.resize(_rhs.n_properties());
            size_ = _rhs.size();
            for (unsigned int i=0; i<_rhs.parrays_.size(); ++i){
                parrays_[i] = _rhs.parrays_[i];
            }
        }
    }
    
    // returns the current size of the property arrays
    size_t size() const { return size_; }

    // returns the number of property arrays
    size_t n_properties() const { return parrays_.size(); }

    // returns a vector of all property names
    std::vector<std::string> properties() const
    {
        std::vector<std::string> names;
        for (unsigned int i=0; i<parrays_.size(); ++i)
            names.push_back(parrays_[i]->name());
        return names;
    }


    // add a property with name \c name and default value \c t
    template <class T> WingedgeProperty<T> add(const std::string& name, const T t=T())
    {
        WingedgeProperty_array<T>* p = new WingedgeProperty_array<T>(name, t);
        p->resize(size_);
        parrays_.push_back(p);
        return WingedgeProperty<T>(p);
    }


    // get a property by its name. returns invalid property if it does not exist.
    template <class T> WingedgeProperty<T> get(const std::string& name) const
    {
        for (unsigned int i=0; i<parrays_.size(); ++i)
            if (parrays_[i]->name() == name)
                return WingedgeProperty<T>(dynamic_cast<WingedgeProperty_array<T>*>(parrays_[i]));
        return WingedgeProperty<T>();
    }

    // Verifies a property of given name & type exists
    template <class T> bool exists(const std::string& name)
    {     
        WingedgeProperty<T> p = get<T>(name);
        return bool(p);
    }
    
    // returns a property if it exists, otherwise it creates it first.
    template <class T> WingedgeProperty<T> get_or_add(const std::string& name, const T t=T())
    {
        WingedgeProperty<T> p = get<T>(name);
        if (!p) p = add<T>(name, t);
        return p;
    }


    // delete a property
    template <class T> void remove(WingedgeProperty<T>& h)
    {
        std::vector<Wingedge_Base_property_array*>::iterator it=parrays_.begin(), end=parrays_.end();
        for (; it!=end; ++it)
        {
            if (*it == h.parray_)
            {
                delete *it;
                parrays_.erase(it);
                h.reset();
                break;
            }
        }
    }


    // delete all properties
    void clear()
    {
        for (unsigned int i=0; i<parrays_.size(); ++i)
            delete parrays_[i];
        parrays_.clear();
        size_ = 0;
    }


    // reserve memory for n entries in all arrays
    void reserve(size_t n) const
    {
        for (unsigned int i=0; i<parrays_.size(); ++i)
            parrays_[i]->reserve(n);
    }

    // resize all arrays to size n
    void resize(size_t n)
    {
        for (unsigned int i=0; i<parrays_.size(); ++i)
            parrays_[i]->resize(n);
        size_ = n;
    }

    // free unused space in all arrays
    void free_memory() const
    {
        for (unsigned int i=0; i<parrays_.size(); ++i)
            parrays_[i]->free_memory();
    }

    // add a new element to each vector
    void push_back()
    {
        for (unsigned int i=0; i<parrays_.size(); ++i)
            parrays_[i]->push_back();
        ++size_;
    }

    // swap elements i0 and i1 in all arrays
    void swap(size_t i0, size_t i1) const
    {
        for (unsigned int i=0; i<parrays_.size(); ++i)
            parrays_[i]->swap(i0, i1);
    }


private:
    std::vector<Wingedge_Base_property_array*>  parrays_;
    size_t  size_;
};

template <class Scalar, class Vector>
class WingedgeMesh {
  
public: //------------------------------------------------------ topology types

    /// Base class for all topology types (internally it is basically an index)
    /// \sa Vertex, Edge, Face
    class Base_handle
    {
    public:

        /// constructor
        explicit Base_handle(int _idx=-1) : idx_(_idx) {}

        /// Get the underlying index of this handle
        int idx() const { return idx_; }

        /// reset handle to be invalid (index=-1)
        void reset() { idx_=-1; }

        /// return whether the handle is valid, i.e., the index is not equal to -1.
        bool is_valid() const { return idx_ != -1; }

        /// are two handles equal?
        bool operator==(const Base_handle& _rhs) const {
            return idx_ == _rhs.idx_;
        }

        /// are two handles different?
        bool operator!=(const Base_handle& _rhs) const {
            return idx_ != _rhs.idx_;
        }

        /// compare operator useful for sorting handles
        bool operator<(const Base_handle& _rhs) const {
            return idx_ < _rhs.idx_;
        }
        
    private:
        friend class Vertex_iterator;
        friend class Halfedge_iterator;
        friend class Edge_iterator;
        friend class Face_iterator;
        friend class WingedgeMesh;
        int idx_;
    };


    /// this type represents a vertex (internally it is basically an index)
    ///  \sa Edge, Face
    struct Vertex : public Base_handle
    {
        /// default constructor (with invalid index)
        explicit Vertex(int _idx=-1) : Base_handle(_idx) {}
    };

    /// this type represents an edge (internally it is basically an index)
    /// \sa Vertex, Face
    struct Edge : public Base_handle
    {
        /// default constructor (with invalid index)
        explicit Edge(int _idx=-1) : Base_handle(_idx) {}
    };


    /// this type represents a face (internally it is basically an index)
    /// \sa Vertex, Edge
    struct Face : public Base_handle
    {
        /// default constructor (with invalid index)
        explicit Face(int _idx=-1) : Base_handle(_idx) {}
    };




public: //-------------------------------------------------- connectivity types

    /// This type stores the vertex connectivity
    struct Vertex_connectivity
    {
        std::set<Edge> edges_;
    };

    /// This type stores the face connectivity
    struct Face_connectivity
    {
        std::set<Vertex> vertices_;
    };

    /// This type stores the edge connectivity
    struct Edge_connectivity
    {
        /// vertex on the edge
        Vertex    vertex0_;
        Vertex    vertex1_;
        
        /// wing faces
        std::set<Face> faces_;
    };

public: //------------------------------------------------------ property types

    /// Vertex property of type T
    /// \sa Halfedge_property, Edge_property, Face_property
    template <class T> class Vertex_property : public WingedgeProperty<T>
    {
    public:

        /// default constructor
        explicit Vertex_property() {}
        explicit Vertex_property(WingedgeProperty<T> p) : WingedgeProperty<T>(p) {}

        /// access the data stored for vertex \c v
        typename WingedgeProperty<T>::reference operator[](Vertex v)
        {
            return WingedgeProperty<T>::operator[](v.idx());
        }

        /// access the data stored for vertex \c v
        typename WingedgeProperty<T>::const_reference operator[](Vertex v) const
        {
            return WingedgeProperty<T>::operator[](v.idx());
        }
    };

    /// Edge property of type T
    /// \sa Vertex_property, Halfedge_property, Face_property
    template <class T> class Edge_property : public WingedgeProperty<T>
    {
    public:

        /// default constructor
        explicit Edge_property() {}
        explicit Edge_property(WingedgeProperty<T> p) : WingedgeProperty<T>(p) {}

        /// access the data stored for edge \c e
        typename WingedgeProperty<T>::reference operator[](Edge e)
        {
            return WingedgeProperty<T>::operator[](e.idx());
        }

        /// access the data stored for edge \c e
        typename WingedgeProperty<T>::const_reference operator[](Edge e) const
        {
            return WingedgeProperty<T>::operator[](e.idx());
        }
    };


    /// Face property of type T
    /// \sa Vertex_property, Halfedge_property, Edge_property
    template <class T> class Face_property : public WingedgeProperty<T>
    {
    public:

        /// default constructor
        explicit Face_property() {}
        explicit Face_property(WingedgeProperty<T> p) : WingedgeProperty<T>(p) {}

        /// access the data stored for face \c f
        typename WingedgeProperty<T>::reference operator[](Face f)
        {
            return WingedgeProperty<T>::operator[](f.idx());
        }

        /// access the data stored for face \c f
        typename WingedgeProperty<T>::const_reference operator[](Face f) const
        {
            return WingedgeProperty<T>::operator[](f.idx());
        }
    };




public: //------------------------------------------------------ iterator types

    /// this class iterates linearly over all vertices
    /// \sa vertices_begin(), vertices_end()
    /// \sa Edge_iterator, Face_iterator
    class Vertex_iterator
    {
    public:

        /// Default constructor
        Vertex_iterator(Vertex v=Vertex(), const WingedgeMesh* m=NULL) : hnd_(v), mesh_(m)
        {
            if (mesh_ && mesh_->garbage()) while (mesh_->is_valid(hnd_) && mesh_->is_deleted(hnd_)) ++hnd_.idx_;
        }

        /// Cast to the vertex the iterator refers to
        operator Vertex() const { return hnd_; }

        /// are two iterators equal?
        bool operator==(const Vertex_iterator& rhs) const
        {
            return (hnd_==rhs.hnd_);
        }

        /// are two iterators different?
        bool operator!=(const Vertex_iterator& rhs) const
        {
            return !operator==(rhs);
        }

        /// pre-increment iterator
        Vertex_iterator& operator++()
        {
            ++hnd_.idx_;
            assert(mesh_);
            while (mesh_->garbage() && mesh_->is_valid(hnd_) && mesh_->is_deleted(hnd_)) ++hnd_.idx_;
            return *this;
        }

        /// pre-decrement iterator
        Vertex_iterator& operator--()
        {
            --hnd_.idx_;
            assert(mesh_);
            while (mesh_->garbage() && mesh_->is_valid(hnd_) && mesh_->is_deleted(hnd_)) --hnd_.idx_;
            return *this;
        }

    private:
        Vertex  hnd_;
        const WingedgeMesh* mesh_;
    };

    /// this class iterates linearly over all edges
    /// \sa edges_begin(), edges_end()
    /// \sa Vertex_iterator, Face_iterator
    class Edge_iterator
    {
    public:

        /// Default constructor
        Edge_iterator(Edge e=Edge(), const WingedgeMesh* m=NULL) : hnd_(e), mesh_(m)
        {
            if (mesh_ && mesh_->garbage()) while (mesh_->is_valid(hnd_) && mesh_->is_deleted(hnd_)) ++hnd_.idx_;
        }

        /// case to the edge the iterator refers to
        operator Edge() const { return hnd_; }

        /// are two iterators equal?
        bool operator==(const Edge_iterator& rhs) const
        {
            return (hnd_==rhs.hnd_);
        }

        /// are two iterators different?
        bool operator!=(const Edge_iterator& rhs) const
        {
            return !operator==(rhs);
        }

        /// pre-increment iterator
        Edge_iterator& operator++()
        {
            ++hnd_.idx_;
            assert(mesh_);
            while (mesh_->garbage() && mesh_->is_valid(hnd_) && mesh_->is_deleted(hnd_)) ++hnd_.idx_;
            return *this;
        }

        /// pre-decrement iterator
        Edge_iterator& operator--()
        {
            --hnd_.idx_;
            assert(mesh_);
            while (mesh_->garbage() && mesh_->is_valid(hnd_) && mesh_->is_deleted(hnd_)) --hnd_.idx_;
            return *this;
        }

    private:
        Edge  hnd_;
        const WingedgeMesh* mesh_;
    };


    /// this class iterates linearly over all faces
    /// \sa faces_begin(), faces_end()
    /// \sa Vertex_iterator, Edge_iterator
    class Face_iterator
    {
    public:

        /// Default constructor
        Face_iterator(Face f=Face(), const WingedgeMesh* m=NULL) : hnd_(f), mesh_(m)
        {
            if (mesh_ && mesh_->garbage()) while (mesh_->is_valid(hnd_) && mesh_->is_deleted(hnd_)) ++hnd_.idx_;
        }

        /// case to the face the iterator refers to
        operator Face() const { return hnd_; }

        /// are two iterators equal?
        bool operator==(const Face_iterator& rhs) const
        {
            return (hnd_==rhs.hnd_);
        }

        /// are two iterators different?
        bool operator!=(const Face_iterator& rhs) const
        {
            return !operator==(rhs);
        }

        /// pre-increment iterator
        Face_iterator& operator++()
        {
            ++hnd_.idx_;
            assert(mesh_);
            while (mesh_->garbage() && mesh_->is_valid(hnd_) && mesh_->is_deleted(hnd_)) ++hnd_.idx_;
            return *this;
        }

        /// pre-decrement iterator
        Face_iterator& operator--()
        {
            --hnd_.idx_;
            assert(mesh_);
            while (mesh_->garbage() && mesh_->is_valid(hnd_) && mesh_->is_deleted(hnd_)) --hnd_.idx_;
            return *this;
        }

    private:
        Face  hnd_;
        const WingedgeMesh* mesh_;
    };

public: //---------------------------------------------------- circulator types
	
	/// this class circulates through all outgoing edges of a vertex
	/// \sa Vertex_around_vertex_circulator, Face_around_vertex_circulator
	class Edge_around_vertex
	{
	public:

		/// default constructor
		Edge_around_vertex(const WingedgeMesh* m=NULL, Vertex v=Vertex()): mesh_(m)
		{
			edge_ = mesh_->vconn_[v].edges_.begin();
			end_ = mesh_->vconn_[v].edges_.end();
		}

		bool end()
		{
			return edge_ == end_;
		}

		/// are two circulators equal?
		bool operator==(const Edge_around_vertex& rhs) const
		{
			return ((mesh_==rhs.mesh_) && (edge_==rhs.edge_));
		}

		/// are two circulators different?
		bool operator!=(const Edge_around_vertex& rhs) const
		{
			return !operator==(rhs);
		}

		/// pre-increment (rotate clockwise)
		Edge_around_vertex& operator++()
		{
			edge_++;
			return *this;
		}

		/// pre-decrement (rotate counter-clockwise)
		Edge_around_vertex& operator--()
		{
			edge_--;
			return *this;
		}

		/// cast to Edge: gives current outgoing halfedge
		operator Edge() const { return *edge_; }

		/// cast to bool: true if vertex is not isolated
		operator bool() const { return mesh_->is_valid(*edge_); }

		operator typename std::set<Edge>::iterator() const { return edge_; }

	private:
		const WingedgeMesh*  mesh_;
		typename std::set<Edge>::iterator edge_, end_;
	};

public: //-------------------------------------------- constructor / destructor

    /// \name Construct, destruct, assignment
    //@{

    /// default constructor
    WingedgeMesh(){
        // allocate standard properties
        // same list is used in operator=() and assign()
        vconn_    = add_vertex_property<Vertex_connectivity>("v:connectivity");
        econn_    = add_edge_property<Edge_connectivity>("e:connectivity");
        fconn_    = add_face_property<Face_connectivity>("f:connectivity");
        vpoint_   = add_vertex_property<Vector>("v:point");
        vdeleted_ = add_vertex_property<bool>("v:deleted", false);
        edeleted_ = add_edge_property<bool>("e:deleted", false);
        fdeleted_ = add_face_property<bool>("f:deleted", false);

        deleted_vertices_ = deleted_edges_ = deleted_faces_ = 0;
        garbage_ = false;
    }

    // destructor
    ~WingedgeMesh(){}
        
    /// assign \c rhs to \c *this. performs a deep copy of all properties.
    //WingedgeMesh& operator=(const WingedgeMesh& rhs);

    /// assign \c rhs to \c *this. does not copy custom properties.
    //WingedgeMesh& assign(const WingedgeMesh& rhs);

    /// assign \c rhs to \c *this. performs a shallow copy of all properties.
    //void shallow_copy(const WingedgeMesh& rhs);
    
    //@}




public: //------------------------------------------------------------- file IO

    /// \name File IO
    //@{

    /// read mesh from file \c filename. file extension determines file type.
    /// \sa write(const std::string& filename)
    //bool read(const std::string& filename);

    /// write mesh to file \c filename. file extensions determines file type.
    /// \sa read(const std::string& filename)
    //bool write(const std::string& filename) const;

    //@}




public: //----------------------------------------------- add new vertex / face

    /// \name Add new elements by hand
    //@{

    /// add a new vertex with position \c p
    Vertex add_vertex(const Vector& p){
        Vertex v = new_vertex();
        vpoint_[v] = p;
        return v;
    }
    
    Edge add_edge(Vertex v0, Vertex v1){
        return new_edge(v0,v1);
    }
        
    /// add a new triangle connecting vertices \c v1, \c v2, \c v3
    /// \sa add_face 
    Face add_triangle(Vertex v0, Vertex v1, Vertex v2){
        std::vector<Vertex> v(3);
        v[0] = v0;
        v[1] = v1;
        v[2] = v2;
        return add_face(v);
    }

    /// add a new face with vertex list \c vertices
    /// \sa add_triangle, add_quad
    Face add_face(const std::vector<Vertex>& vertices){

        int N = vertices.size();

        // Create the face
        Face f(new_face());

        // Add vertices to face
        for(int i = 0; i < N; i++)
            set_vertex(f, vertices[i]);

        // Add edges if needed, or get old ones
        std::vector<Edge> edges;
        for(int i = 0; i < N; i++){
            int j = ((i + 1) % N);
            edges.push_back(add_edge(vertices[i], vertices[j]));
        }

        // Add face to edges
        for(int e = 0; e < (int) edges.size(); e++)
            set_face(edges[e], f);

        return f;
    }

    //@}




public: //--------------------------------------------------- memory management

    /// \name Memory Management
    //@{

    /// returns number of (deleted and valid) vertices in the mesh
    unsigned int vertices_size() const { return (unsigned int) vprops_.size(); }
    /// returns number of (deleted and valid)edges in the mesh
    unsigned int edges_size() const { return (unsigned int) eprops_.size(); }
    /// returns number of (deleted and valid)faces in the mesh
    unsigned int faces_size() const { return (unsigned int) fprops_.size(); }


    /// returns number of vertices in the mesh
    unsigned int n_vertices() const { return vertices_size() - deleted_vertices_; }
    /// returns number of edges in the mesh
    unsigned int n_edges() const { return edges_size() - deleted_edges_; }
    /// returns number of faces in the mesh
    unsigned int n_faces() const { return faces_size() - deleted_faces_; }


    /// returns true iff the mesh is empty, i.e., has no vertices
    unsigned int empty() const { return n_vertices() == 0; }


    /// clear mesh: remove all vertices, edges, faces
    void clear(){
        vprops_.resize(0);
        eprops_.resize(0);
        fprops_.resize(0);
    
        vprops_.free_memory();
        eprops_.free_memory();
        fprops_.free_memory();
    
        deleted_vertices_ = deleted_edges_ = deleted_faces_ = 0;
        garbage_ = false;
    }

    /// remove unused memory from vectors
    void free_memory(){
        vprops_.free_memory();
        eprops_.free_memory();
        fprops_.free_memory();
    }

    /// reserve memory (mainly used in file readers)
    void reserve(unsigned int nvertices, unsigned int nedges, unsigned int nfaces ){
        vprops_.reserve(nvertices);
        eprops_.reserve(nedges);
        fprops_.reserve(nfaces);
    }

    /// remove deleted vertices/edges/[faces: not implemented]
    void garbage_collection()
	{
        std::map<int, int> points;
        std::vector< Vector > points_pos;
        std::vector< std::pair<int,int> > edges;

        int vidx = 0;

        // Collect living vertices
        for(Vertex_iterator vi = vertices_begin(); vi != vertices_end(); ++vi )
        {
            Vertex v = vi;
            if(is_deleted(v) || !is_valid(v)) continue;

            points[v.idx()] = vidx++;
            points_pos.push_back(vpoint_[v]);
        }

        // Collect living edges
        for(Edge_iterator ei = edges_begin(); ei != edges_end(); ++ei)
        {
            Edge e = ei;

            Vertex v0 = vertex(e,0);
            Vertex v1 = vertex(e,1);

            if(!is_deleted(e) && !is_deleted(v0) && !is_deleted(v1) && v0 != v1)
                edges.push_back(std::pair<int,int> (points[v0.idx()], points[v1.idx()]) );
        }

        // Remove old records
        clear();

        // Add clean structure:

		// Vertices
        for(unsigned int i = 0; i < points_pos.size(); i++)
            this->add_vertex(points_pos[i]);

		// Edges
        for(unsigned int i = 0; i < edges.size(); i++)
            this->add_edge(Vertex(edges[i].first), Vertex(edges[i].second));
    }


    /// returns whether vertex \c v is deleted
    /// \sa garbage_collection()
    bool is_deleted(Vertex v) const
    {
        return vdeleted_[v];
    }
    /// returns whether edge \c e is deleted
    /// \sa garbage_collection()
    bool is_deleted(Edge e) const
    {
        return edeleted_[e];
    }
    /// returns whether face \c f is deleted
    /// \sa garbage_collection()
    bool is_deleted(Face f) const
    {
        return fdeleted_[f];
    }


    /// return whether vertex \c v is valid, i.e. the index is stores it within the array bounds.
    bool is_valid(Vertex v) const
    {
        return (0 <= v.idx()) && (v.idx() < (int)vertices_size());
    }
    /// return whether edge \c e is valid, i.e. the index is stores it within the array bounds.
    bool is_valid(Edge e) const
    {
        return (0 <= e.idx()) && (e.idx() < (int)edges_size());
    }
    /// return whether face \c f is valid, i.e. the index is stores it within the array bounds.
    bool is_valid(Face f) const
    {
        return (0 <= f.idx()) && (f.idx() < (int)faces_size());
    }

    //@}




public: //---------------------------------------------- low-level connectivity

    /// \name Low-level connectivity
    //@{

    void set_vertex(Face f, Vertex v)
    {
        fconn_[f].vertices_.insert(v);
    }

    void set_edge(Vertex v, Edge e)
    {
        vconn_[v].edges_.insert(e);
    }

    void set_face(Edge e, Face f)
    {
        econn_[e].faces_.insert(f);
    }
	
    bool edge_connects(Edge e, Vertex v)
    {
        if(econn_[e].vertex0_ == v || econn_[e].vertex1_ == v)
            return true;
        else
            return false;
    }

	bool has_faces(Edge e)
	{
		// Check if any faces are not deleted
		for(typename std::set<Face>::iterator fit = econn_[e].faces_.begin(); fit != econn_[e].faces_.end(); fit++)
		{
			if(!fdeleted_[*fit])
				return true;
		}

		return false;
	}

    /// returns whether \c v is a boundary vertex
    bool is_boundary(Vertex /*v*/) const
    {
        throw StarlabException("is_boundary(Vertex v) NOT IMPLEMENTED");
        return false;
    }

    /// returns whether \c v is isolated, i.e., not incident to any face
    bool is_isolated(Vertex /*v*/) const
    {
        throw StarlabException("is_isolated(Vertex v) NOT IMPLEMENTED");
        return false;
    }

    /// returns the \c i'th vertex of edge \c e. \c i has to be 0 or 1.
    Vertex vertex(Edge e, unsigned int i) const
    {
        assert(i<=1 /*&& i>=0 unnecessary because unsigned*/);
        if(i==0) return econn_[e].vertex0_;
        if(i==1) return econn_[e].vertex1_;
        return Vertex();
    }

    /// returns whether \c e is a boundary edge, i.e., if one of its
    /// halfedges is a boundary halfedge.
    bool is_boundary(Edge /*e*/) const
    {
        throw StarlabException("bool is_boundary(Edge e) const NOT IMPLEMENTED");
        return false;
    }

    /// returns whether \c f is a boundary face, i.e., it one of its edges is a boundary edge.
    bool is_boundary(Face /*f*/) const
    {
        throw StarlabException("bool is_boundary(Face /*f*/) NOT IMPLEMENTED");
        return false;
    }

    //@}


public: //--------------------------------------------------- property handling

    /// \name WingedgeProperty handling
    //@{

    /** add a vertex property of type \c T with name \c name and default value \c t.
     fails if a property named \c name exists already, since the name has to be unique.
     in this case it returns an invalid property */
    template <class T> Vertex_property<T> add_vertex_property(const std::string& name, const T t=T())
    {
        return Vertex_property<T>(vprops_.add<T>(name, t));
    }
    /** add a edge property of type \c T with name \c name and default value \c t.
     fails if a property named \c name exists already, since the name has to be unique.
     in this case it returns an invalid property */
    template <class T> Edge_property<T> add_edge_property(const std::string& name, const T t=T())
    {
        return Edge_property<T>(eprops_.add<T>(name, t));
    }
    /** add a face property of type \c T with name \c name and default value \c t.
     fails if a property named \c name exists already, since the name has to be unique.
     in this case it returns an invalid property */
    template <class T> Face_property<T> add_face_property(const std::string& name, const T t=T())
    {
        return Face_property<T>(fprops_.add<T>(name, t));
    }


    /** get the vertex property named \c name of type \c T. returns an invalid
     Vertex_property if the property does not exist or if the type does not match. */
    template <class T> Vertex_property<T> get_vertex_property(const std::string& name) const
    {
        return Vertex_property<T>(vprops_.get<T>(name));
    }
    /** get the edge property named \c name of type \c T. returns an invalid
     Vertex_property if the property does not exist or if the type does not match. */
    template <class T> Edge_property<T> get_edge_property(const std::string& name) const
    {
        return Edge_property<T>(eprops_.get<T>(name));
    }
    /** get the face property named \c name of type \c T. returns an invalid
     Vertex_property if the property does not exist or if the type does not match. */
    template <class T> Face_property<T> get_face_property(const std::string& name) const
    {
        return Face_property<T>(fprops_.get<T>(name));
    }

    /** checks if a face property of type \c T with name \c name exists. */
    template <class T> bool has_face_property(const std::string& name)
    {
        return fprops_.exists<T>(name);
    } 
    
    /** checks if a vertex property of type \c T with name \c name exists. */
    template <class T> bool has_vertex_property(const std::string& name)
    {
        return vprops_.exists<T>(name);
    }    

    /** if a vertex property of type \c T with name \c name exists, it is returned.
     otherwise this property is added (with default value \c t) */
    template <class T> Vertex_property<T> vertex_property(const std::string& name, const T t=T())
    {
        return Vertex_property<T>(vprops_.get_or_add<T>(name, t));
    }
    
    /** checks if a vertex property of type \c T with name \c name exists. */
    template <class T> bool has_edge_property(const std::string& name)
    {
        return eprops_.exists<T>(name);
    }
    
    /** if an edge property of type \c T with name \c name exists, it is returned.
     otherwise this property is added (with default value \c t) */
    template <class T> Edge_property<T> edge_property(const std::string& name, const T t=T())
    {
        return Edge_property<T>(eprops_.get_or_add<T>(name, t));
    }
    /** if a face property of type \c T with name \c name exists, it is returned.
     otherwise this property is added (with default value \c t) */
    template <class T> Face_property<T> face_property(const std::string& name, const T t=T())
    {
        return Face_property<T>(fprops_.get_or_add<T>(name, t));
    }


    /// remove the vertex property \c p
    template <class T> void remove_vertex_property(Vertex_property<T>& p)
    {
        vprops_.remove(p);
    }
    /// remove the edge property \c p
    template <class T> void remove_edge_property(Edge_property<T>& p)
    {
        eprops_.remove(p);
    }
    /// remove the face property \c p
    template <class T> void remove_face_property(Face_property<T>& p)
    {
        fprops_.remove(p);
    }


    /// returns the names of all vertex properties
    std::vector<std::string> vertex_properties() const
    {
        return vprops_.properties();
    }
    /// returns the names of all edge properties
    std::vector<std::string> edge_properties() const
    {
        return eprops_.properties();
    }
    /// returns the names of all face properties
    std::vector<std::string> face_properties() const
    {
        return fprops_.properties();
    }
    /// prints the names of all properties
    void property_stats() const{
#if 0
        std::vector<std::string> props;
    
        std::cout << "vertex properties:\n";
        props = vertex_properties();
        for (unsigned int i=0; i<props.size(); ++i)
            std::cout << "\t" << props[i] << std::endl;
    
        std::cout << "edge properties:\n";
        props = edge_properties();
        for (unsigned int i=0; i<props.size(); ++i)
            std::cout << "\t" << props[i] << std::endl;
    
        std::cout << "face properties:\n";
        props = face_properties();
        for (unsigned int i=0; i<props.size(); ++i)
            std::cout << "\t" << props[i] << std::endl;        
#endif
    }

    //@}




public: //--------------------------------------------- iterators & circulators

    /// \name Iterators & Circulators
    //@{

    /// returns start iterator for vertices
    Vertex_iterator vertices_begin() const
    {
        return Vertex_iterator(Vertex(0), this);
    }

    /// returns end iterator for vertices
    Vertex_iterator vertices_end() const
    {
        return Vertex_iterator(Vertex(vertices_size()), this);
    }

    /// returns start iterator for edges
    Edge_iterator edges_begin() const
    {
        return Edge_iterator(Edge(0), this);
    }

    /// returns end iterator for edges
    Edge_iterator edges_end() const
    {
        return Edge_iterator(Edge(edges_size()), this);
    }

    /// returns start iterator for faces
    Face_iterator faces_begin() const
    {
        return Face_iterator(Face(0), this);
    }

    /// returns end iterator for faces
    Face_iterator faces_end() const
    {
        return Face_iterator(Face(faces_size()), this);
    }

    //@}

public: //--------------------------------------------- higher-level operations

    /// \name Higher-level Topological Operations
    /// @{
    void collapse(Edge e)
	{
		// remove edge
		remove_edge(e);
	}
    /// @}

    unsigned int valence(Vertex v) const
    {
        return vconn_[v].edges_.size();
    }


public: //------------------------------------------ geometry-related functions

	Scalar edge_length(Edge e) const
	{
		return (vpoint_[vertex(e,0)] - vpoint_[vertex(e,1)]).norm();
	}


	void print_edges(Vertex v)
	{
		printf("Vertex (%d) has edges: {", v.idx());

		for(typename std::set<Edge>::iterator eit = vconn_[v].edges_.begin(); eit != vconn_[v].edges_.end(); eit++){
			printf("%d, ", eit->idx());
		}

		printf("}\n");
	}

	void print_edge_faces(Vertex v)
	{
		printf("Vertex (%d) has edges: {{{{", v.idx());

		for(typename std::set<Edge>::iterator eit = vconn_[v].edges_.begin(); eit != vconn_[v].edges_.end(); eit++)
		{
			printf("\t");
			print_faces(*eit);
		}

		printf("}}}}\n");
	}

	void print_vertices(Edge e)
	{
		printf("Edge (%d) has: { v0 = %d  |  v1 = %d }\n", e.idx(), econn_[e].vertex0_.idx(), econn_[e].vertex1_.idx());
	}

	void print_faces(Edge e)
	{
		printf("Edge (%d) has faces: {", e.idx());

		for(typename std::set<Face>::iterator fit = econn_[e].faces_.begin(); fit != econn_[e].faces_.end(); fit++)
		{
			printf("%d, ", fit->idx());
		}

		printf("}\n");
	}

	void print_stats()
	{
		printf("\nNumber of vertices \t= %d \n", n_vertices());
		printf("Number of faces \t= %d \n", n_faces());
        printf("Number of edges \t= %d \t (out of %d) \n\n", n_edges(), edges_size());
	}
	
protected: //---------------------------------------------- allocate new elements

    /// allocate a new vertex, resize vertex properties accordingly.
    Vertex new_vertex()
    {
        vprops_.push_back();
        return Vertex(vertices_size()-1);
    }

    /// allocate a new edge, resize edge properties accordingly.
    Edge new_edge(Vertex start, Vertex end){
        assert(start != end);

        // If edge exists already, return it
        Edge pastEdge = get_edge(start, end);
        if(pastEdge.idx() >= 0) return pastEdge;

        eprops_.push_back();
        Edge e(edges_size()-1);
        econn_[e].vertex0_ = start;
        econn_[e].vertex1_ = end;

        // Add edge to both vertices
        set_edge(start, e);
        set_edge(end, e);

        return e;
    }

    /// allocate a new face, resize face properties accordingly.
    Face new_face()
    {
        fprops_.push_back();
        return Face(faces_size()-1);
    }


public: //--------------------------------------------------- helper functions

    /// Helper for halfedge collapse
    void remove_edge(Edge e)
	{
		Vertex v1 = vertex(e, 0);  // 'v1' will be deleted
		Vertex v2 = vertex(e, 1);

		// Remove edge from vertex connectivity list
		remove_edge(v1, e);
		remove_edge(v2, e);

		// Replace 'v1' in edges of 'v1' -> 'v2'
		for(typename std::set<Edge>::iterator eit = vconn_[v1].edges_.begin(); eit != vconn_[v1].edges_.end(); eit++){
			replace_vertex(*eit, v1, v2);

			// connect to new vertex
			set_edge(v2, *eit);
		}

		// Delete dead faces of removed edge 'e'
		std::set<Face> deadFaces = econn_[e].faces_;

		// Remove the duplicated edges
		std::vector<Edge> v2_adj = edges_of(v2);

		for(uint i = 0; i < v2_adj.size(); i++)
		{
			// We will kill 'ei'
			Edge ei = v2_adj[i];

			for(uint j = i + 1; j < v2_adj.size(); j++)
			{
				Edge ej = v2_adj[j];

				if( same_edge(ei, ej) )
				{
					// Migrate faces of 'ei'
					std::set<Face> oldFaces = econn_[ei].faces_;

					for(typename std::set<Face>::iterator fit = oldFaces.begin(); fit != oldFaces.end(); fit++)
					{
						if(deadFaces.find(*fit) == deadFaces.end())
							set_face(ej, *fit);
					}

					// Delete the duplicated edge
					remove_edge(v2, ei);
					edeleted_[v2_adj[i]] = true; 
					++deleted_edges_;
				}
			}
		}

		// Not sure why this is needed since defined in constructor..
		// (just following Surface_mesh for now)
		reserve_delete();

		// delete stuff
		vdeleted_[v1]   = true; ++deleted_vertices_;


		for(typename std::set<Face>::iterator fit = deadFaces.begin(); fit != deadFaces.end(); fit++)
		{
			if(!fdeleted_[*fit])
			{
				fdeleted_[*fit] = true; 
				++deleted_faces_;
			}
		}

		garbage_ = true;

		vertex_collapsed = v1;
	}

	bool same_edge(Edge e1, Edge e2)
	{
		// first edge
		Vertex v1 = econn_[e1].vertex0_;
		Vertex v2 = econn_[e1].vertex1_;

		// second edge
		Vertex v3 = econn_[e2].vertex0_;
		Vertex v4 = econn_[e2].vertex1_;

		return ((v1 == v3) || (v1 == v4)) && ((v2 == v3) || (v2 == v4));
	}

	std::vector<Edge> edges_of(Vertex v)
	{
		std::vector<Edge> result;

		for(typename std::set<Edge>::iterator ei = vconn_[v].edges_.begin(); 
			ei != vconn_[v].edges_.end(); ei++)
			result.push_back(*ei);

		return result;
	}

	void remove_edge(Vertex v, Edge e)
	{
		vconn_[v].edges_.erase(e);
	}

	void delete_edge(Edge e)
	{
		if(!edeleted_[e])
		{
			edeleted_[e]	= true; 
			++deleted_edges_;
		}
	}

	void replace_vertex(Edge e, Vertex from, Vertex to)
	{
		if(econn_[e].vertex0_ == from) econn_[e].vertex0_ = to;
		if(econn_[e].vertex1_ == from) econn_[e].vertex1_ = to;
	}

	void replace_face(Edge e, Face oldFace, Face newFace)
	{
		econn_[e].faces_.erase(oldFace);
		econn_[e].faces_.insert(newFace);
	}

    /// find the edge from start to end
	Edge get_edge(Vertex start, Vertex end)
	{
		for(typename std::set<Edge>::iterator eit = vconn_[start].edges_.begin(); eit != vconn_[start].edges_.end(); eit++){
			if(edge_connects(*eit, start) && edge_connects(*eit, end))
				return *eit;
		}
		return Edge();
	}

	void reserve_delete()
	{
        if (!vdeleted_) vdeleted_ = vertex_property<bool>("v:deleted", false);
        if (!edeleted_) edeleted_ = edge_property<bool>("e:deleted", false);
        if (!fdeleted_) fdeleted_ = face_property<bool>("f:deleted", false);
	}

public:  //--------------------------------------------------- public helper functions
	Vertex other_vertex(Edge e, Vertex v)
	{
		if(v == econn_[e].vertex0_) 
			return econn_[e].vertex1_;
		else
			return econn_[e].vertex0_; 
	}

	std::set<Vertex> vertex_neighbours(Vertex v)
	{
		std::set<Vertex> result;
		
		for(typename std::set<Edge>::iterator eit = vconn_[v].edges_.begin(); eit != vconn_[v].edges_.end(); eit++)
		{
			Edge e = *eit;
			result.insert(other_vertex(e, v));
		}

		return result;
	}

	int num_edges(Vertex v)
	{
		return vconn_[v].edges_.size();
	}

public:
    /// are there deleted vertices, edges or faces?
    bool garbage() const { return garbage_; }

	Vertex vertex_collapsed; // removed vertex from latest collapsed edge

protected: //------------------------------------------------------- private data

    Property_container vprops_;
    Property_container eprops_;
    Property_container fprops_;

    Vertex_property<Vertex_connectivity>      vconn_;
    Face_property<Face_connectivity>          fconn_;
    Edge_property<Edge_connectivity>          econn_;


    Vertex_property<bool>  vdeleted_;
    Edge_property<bool>    edeleted_;
    Face_property<bool>    fdeleted_;

    Vertex_property<Vector>   vpoint_;

    unsigned int deleted_vertices_;
    unsigned int deleted_edges_;
    unsigned int deleted_faces_;
    bool garbage_;

};

/// @}

} // namespace
