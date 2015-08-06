// Adapted from VCGLib
#pragma once
#define _USE_MATH_DEFINES
#include <math.h>

#include "NanoKdTree.h"
#include "SurfaceMeshHelper.h"

class BallPivoting{
	
	/* Ball pivoting surface reconsturction algorithm:
	1) the vertices used in the new mesh are marked as visited
	2) the border vertices of the new mesh are marked as border
	3) the vector nb is used to keep track of the number of borders a vertex belongs to
	4) usedBit flag is used to select the points in the mesh already processed */   

	struct SimpleFace{ 
		int v[3];
		int & operator[](const int& vi) { return v[vi]; } 
		int & operator()(const int& vi) { return v[vi]; }
		int & V(int k){ return v[k]; }
		int & V0(int k){ return v[(k + 0) % 3]; }
		int & V1(int k){ return v[(k + 1) % 3]; }
		int & V2(int k){ return v[(k + 2) % 3]; }
	};

	// Structure for advancing fronts
	class FrontEdge { 
	public:       
		int v0, v1, v2;		// v0, v1 represent the FrontEdge, v2 the other vertex 
		// in the face this FrontEdge belongs to    
		int face;			// index of the face             
		bool active;		// keep tracks of whether it is in front or in deads

		//the loops in the front are maintained as a Scalar linked list
		std::list<FrontEdge>::iterator next;            
		std::list<FrontEdge>::iterator previous;

		FrontEdge() {}
		FrontEdge(int _v0, int _v1, int _v2, int _face): 
		v0(_v0), v1(_v1), v2(_v2), face(_face), active(true) 
		{ assert(v0 != v1 && v1 != v2 && v0 != v2);}

		const bool operator==(const FrontEdge& f) const
		{ return ((v0 == f.v0) && (v1 == f.v1) && (v2 == f.v2) && (face == f.face)); }
	};  
	enum ListID {FRONT,DEADS};
	typedef std::pair< ListID,std::list<FrontEdge>::iterator > ResultIterator;

	std::list<FrontEdge> front;   
	std::list<FrontEdge> deads;
	std::vector<int> nb; //number of fronts a vertex is into,
	//this is used for the Visited and Border flags
	//but adding topology may not be needed anymore

	SurfaceMeshModel * mesh;        // this structure will be filled by the algorithm
	NanoKdTree * tree;				// lookup structure for points
	std::vector<SimpleFace> fV;		// Output: new face connectivity

	Scalar radius;			// radius of the ball
	Scalar min_edge;			// min length of an edge 
	Scalar max_edge;			// min length of an edge 
	Scalar max_angle;		// max angle between 2 faces (cos(angle) actually)  

	int last_seed;			// used for new seeds when front is empty
	Vector3 baricenter;		// used for the first seed.

	Vector3VertexProperty points;
	BoolVertexProperty visited;
	BoolVertexProperty used;
	BoolVertexProperty border;

public:
	BallPivoting(SurfaceMeshModel  *_mesh, Scalar _radius = -1, Scalar minr = 0.2, Scalar angle = M_PI/2) :
	  mesh(_mesh), radius(_radius), min_edge(minr), max_edge(1.8), max_angle(cos(angle)), last_seed(-1){

		  points = mesh->get_vertex_property<Vector3>(VPOINT);

		  nb.clear();
		  nb.resize(mesh->n_vertices(), 0);

		  // compute bbox and mesh baricenter
		  baricenter = Vector3(0, 0, 0);
		  mesh->updateBoundingBox();
		  foreach(Vertex v, mesh->vertices())
			  baricenter += points[v];
		  baricenter /= this->mesh->n_vertices();

		  assert(this->mesh->n_vertices() > 3);

		  // Initialize KD-tree
		  tree = new NanoKdTree;
		  foreach(Vertex v, mesh->vertices()) tree->addPoint(points[v]);
		  tree->build();

		  // if radius < 0 sample data, if radius == 0 an autoguess
		  // otherwise the passed value (in absolute mesh units) is used.
		  if(radius < 0) {
			  int num_samples = qMin(mesh->n_vertices() * 0.10, 1e5);
			  int k = 7;
			  double maxDist = 0;

			  KDResults matches;
			  for(int a = 0; a < num_samples; a++)
			  {
				  Vertex rand_vert(Vertex( uniform(0, mesh->n_vertices() - 1) ));
				  tree->k_closest(points[rand_vert], k, matches);

				  foreach(KDResultPair r, matches)
					maxDist = qMax(maxDist, r.second);
			  }

			  radius = maxDist;
		  }
		  if(radius == 0)
			  radius = sqrt( pow(mesh->bbox().size().length(), 2) / this->mesh->n_vertices() );

		  min_edge *= radius;
		  max_edge *= radius;    

		  // mark visited points
		  visited = mesh->vertex_property<bool>("v:visited", false);
		  used = mesh->vertex_property<bool>("v:used", false);
		  border = mesh->vertex_property<bool>("v:border", false);
	  }

	  void BuildMesh(int stopAtFace = -1) 
	  {
		  while(true) 
		  {
			  if(!front.size() && !SeedFace()) break;
			  AddFace();

              if(stopAtFace > 0 && (int)fV.size() >= stopAtFace) break;
		  }

		  // Add built faces
		  for(int i = 0; i < (int) fV.size(); i++)
			  mesh->add_triangle(Vertex(fV[i][0]), Vertex(fV[i][1]), Vertex(fV[i][2]));
	  }                          

protected:

	bool SeedFace() {
		int v[3];
		bool success = Seed(v[0], v[1], v[2]);
		if(!success) return false;

		nb.resize(mesh->n_vertices(), 0);

		//create the border of the first face  
		std::list<FrontEdge>::iterator e = front.end();
		std::list<FrontEdge>::iterator last = e;
		std::list<FrontEdge>::iterator first;

		for(int i = 0; i < 3; i++) {
			int v0 = v[i];
			int v1 = v[((i+1)%3)];
			int v2 = v[((i+2)%3)];

			border[Vertex(v0)] = true;

			nb[v[i]]++;

			e = front.insert(front.begin(), FrontEdge(v0, v1, v2, fV.size()));
			if(i != 0) {
				(*last).next = e;    
				(*e).previous = last;
			} else
				first = e;

			last = e;
		} 
		//connect last and first
		(*last).next = first;
		(*first).previous = last;

		AddFace(v[0], v[1], v[2]);
		return true;
	}

	bool Seed(int &v0, int &v1, int &v2) 
	{               
		while(++last_seed < (int)(this->mesh->n_vertices())) 
		{
			Vertex seed (last_seed);
			if(used[seed]) continue;                      

			used[seed] = true;

			// get a sphere of neighbors
			KDResults matches;
			int n = tree->ball_search(points[seed], 2*radius, matches);

			if(n < 3)  
				continue;

			bool success = true;
			//find the closest visited or boundary
			for(int i = 0; i < n; i++) {         
				Vertex v(matches[i].first);
				if(visited[v]) {        
					success = false;
					break;
				}
			}
			if(!success) continue;

			Vertex vv0, vv1, vv2;
			success = false;

			//find a triplet that does not contains any other point
			Vector3 center;
			for(int i = 0; i < n; i++) {
				vv0 = Vertex(matches[i].first);
				Vector3 p0 = points[vv0];        

				for(int k = i+1; k < n; k++) {
					vv1 = Vertex(matches[k].first);          
					Vector3 p1 = points[vv1];      

					Scalar d2 = (p1 - p0).norm();    
					if(d2 < min_edge || d2 > max_edge) continue;

					for(int j = k+1; j < n; j++) {
						vv2 = Vertex(matches[j].first);
						Vector3 p2 = points[vv2];     

						Scalar d1 = (p2 - p0).norm();
						if(d1 < min_edge || d1 > max_edge) continue;            
						Scalar d0 = (p2 - p1).norm();
						if(d0 < min_edge || d0 > max_edge) continue;

						Vector3 normal = cross((p1 - p0),(p2 - p0));

						if(dot(normal, p0 - baricenter) < 0) continue;

						if(!FindSphere(p0, p1, p2, center)) {
							continue;
						}

						//check no other point inside
						int t;
						for(t = 0; t < n; t++) {
							Vector3 tp = points[Vertex(matches[t].first)];
							Scalar dist = (center - tp).norm();
							if((int)(dist * 1e6) < (int)(radius * 1e6))
								break;
						}
						if(t < n) {
							continue;                         
						}

						//check on the other side there is not a surface
						Vector3 opposite = center + normal * ( (dot(center - p0,(normal)) * 2) / normal.sqrnorm() );
						for(t = 0; t < n; t++) {
							Vertex v (matches[t].first);

							Scalar dist = (opposite - points[v]).norm();

							if(visited[v] && dist <= radius)
								break;   
						}
						if(t < n) {
							continue;                         
						}
						success = true;
						i = k = j = n;
					}
				}
			}

			if(!success) { //see bad luck above
				continue;
			}
			Mark(vv0);
			Mark(vv1);
			Mark(vv2);            

			v0 = vv0.idx();
			v1 = vv1.idx();
			v2 = vv2.idx();            
			return true;      
		}
		return false;    
	}

	bool AddFace() {
		if(!front.size()) return false; 

		std::list<FrontEdge>::iterator ei = front.begin();
		FrontEdge &current = *ei;
		FrontEdge &previous = *current.previous;           
		FrontEdge &next = *current.next;  

		int v0 = current.v0, v1 = current.v1;
		assert(nb[v0] < 10 && nb[v1] < 10);

		ResultIterator touch;
		touch.first = FRONT;
		touch.second = front.end();

		int v2 = Place(current, touch);

		if(v2 == -1) {
			KillEdge(ei);
			return false;
		}

		assert(v2 != v0 && v2 != v1);  

		if ((touch.first == FRONT) && (touch.second != front.end()) ||
			(touch.first == DEADS) && (touch.second != deads.end()))

		{  
			//check for orientation and manifoldness    

			//touch == current.previous?  
			if(v2 == previous.v0) {   
				if(!CheckEdge(v2, v1)) {
					KillEdge(ei);
					return false;
				}       

				Detach(v0);

				std::list<FrontEdge>::iterator up = NewEdge(FrontEdge(v2, v1, v0, fV.size()));
				MoveFront(up);
				(*up).previous = previous.previous;
				(*up).next = current.next;
				(*previous.previous).next = up;
				next.previous = up;
				Erase(current.previous);
				Erase(ei);
				Glue(up);

				//touch == (*current.next).next         
			} else if(v2 == next.v1) {    
				if(!CheckEdge(v0, v2)) {
					KillEdge(ei);
					return false;
				}     


				Detach(v1);
				std::list<FrontEdge>::iterator up = NewEdge(FrontEdge(v0, v2, v1, fV.size()));
				MoveFront(up);
				(*up).previous = current.previous;
				(*up).next = (*current.next).next;
				previous.next = up;
				(*next.next).previous = up;
				Erase(current.next);
				Erase(ei);
				Glue(up);
			} else {
				if(!CheckEdge(v0, v2) || !CheckEdge(v2, v1)) {
					KillEdge(ei);
					return false;
				} 

				std::list<FrontEdge>::iterator left = touch.second;
				std::list<FrontEdge>::iterator right = (*touch.second).previous;      

				//this would be a really bad join
				if(v1 == (*right).v0 || v0 == (*left).v1) {
					KillEdge(ei);
					return false;
				}

				nb[v2]++;    

				std::list<FrontEdge>::iterator down = NewEdge(FrontEdge(v2, v1, v0, fV.size()));      
				std::list<FrontEdge>::iterator up = NewEdge(FrontEdge(v0, v2, v1, fV.size()));                            

				(*right).next = down;
				(*down).previous = right;

				(*down).next = current.next;
				next.previous = down;      

				(*left).previous = up;
				(*up).next = left;

				(*up).previous = current.previous;
				previous.next = up;
				Erase(ei);
			}                         


		} 
		else if ((touch.first == FRONT) && (touch.second == front.end()) || 
			(touch.first == DEADS) && (touch.second == deads.end()))
		{
			Vertex vv2 (v2);

			assert(! border[vv2]); //fatal error! a new point is already a border?
			nb[v2]++;                 
			border[vv2] = true;

			std::list<FrontEdge>::iterator down = NewEdge(FrontEdge(v2, v1, v0, fV.size()));
			std::list<FrontEdge>::iterator up = NewEdge(FrontEdge(v0, v2, v1, fV.size()));                        

			(*down).previous = up;
			(*up).next = down;
			(*down).next = current.next;
			next.previous = down;
			(*up).previous = current.previous;
			previous.next = up;
			Erase(ei);
		}

		AddFace(v0, v2, v1);
		return false;
	}       

	void AddFace(int v0, int v1, int v2)
	{
		assert(v0 < (int)mesh->n_vertices() && v1 < (int)mesh->n_vertices() && v2 < (int)mesh->n_vertices()); 
		SimpleFace face;

		face.V(0) = v0;
		face.V(1) = v1;
		face.V(2) = v2;

		//qDebug() << "F" << fV.size() << " :" << v0 << " | " << v1  << " | " << v2;

		fV.push_back(face);
	}
	
	//select a new vertex, mark as Visited and mark as usedBit all neighbors (less than min_edge)
	int Place(FrontEdge &edge, ResultIterator &touch) {
		Vector3 v0 = points[Vertex(edge.v0)];
		Vector3 v1 = points[Vertex(edge.v1)];  
		Vector3 v2 = points[Vertex(edge.v2)];  

		Vector3 normal = cross((v1 - v0),(v2 - v0)).normalize();        
		Vector3 middle = (v0 + v1)/2;    
		Vector3 center;    

		if(!FindSphere(v0, v1, v2, center)) {
			//      assert(0);
			return -1;
		}
		
		Vector3 start_pivot = center - middle;          
		Vector3 axis = (v1 - v0);

		Scalar axis_len = axis.sqrnorm();
		if(axis_len > 4*radius*radius) {
			return -1;
		}
		axis.normalize();

		// r is the radius of the through of all possible spheres passing through v0 and v1
		Scalar r = sqrt(radius*radius - axis_len/4);

		// get a sphere of neighbors
		KDResults matches;
		int n = tree->ball_search( middle, r + radius, matches);

		if(n == 0) {
			return -1; //this really would be strange but one never knows.
		}

		Vertex candidate;
		Scalar min_angle = M_PI;

		for(int i = 0; i < (int)matches.size(); i++) 
		{      
			Vertex v (matches[i].first);
			int id = v.idx();

			// this should always be true IsB => IsV , IsV => IsU
			if(border[v]) assert(visited[v]);
			if(visited[v]) assert(used[v]);

			if(used[v] && !border[v]) continue;
			if(id == edge.v0 || id == edge.v1 || id == edge.v2) continue;

			Vector3 p = points[Vertex(id)];

			/* Find the sphere through v0, p, v1 (store center on end_pivot) */
			if(!FindSphere(v0, p, v1, center)) {
				continue;      
			}

			/* Angle between old center and new center */
			Scalar alpha = Angle(start_pivot, center - middle, axis);

			if(!candidate.is_valid() || alpha < min_angle) {
				candidate = v;
				min_angle = alpha;
			} 
		}
		if(min_angle >= M_PI - 0.1) {
			return -1;
		}

		if(!candidate.is_valid()) {
			return -1;
		}
		if(!border[candidate]) {
			assert((points[candidate] - v0).norm() > min_edge);
			assert((points[candidate] - v1).norm() > min_edge);    
		}

		int id = candidate.idx();
		assert(id != edge.v0 && id != edge.v1);

		Vector3 newnormal = cross((points[candidate] - v0),(v1 - v0)).normalize();
		if(dot(normal, newnormal) < max_angle || this->nb[id] >= 2) {
			return -1;
		}

		Vector3 mid = (points[candidate] + v0) / 2;

		// Experimental: this seem to resolve an issue that seem to be numerical 
		// Solution: discard samples almost on edge of possible triangles
		KDResults match;
		int n2 = tree->ball_search(mid, min_edge * 0.5, match);
        for(int m = 0; m < n2; m++)
			used[Vertex(match[m].first)] = true;

		//test if id is in some border (to return touch)
		for(std::list<FrontEdge>::iterator k = this->front.begin(); k != this->front.end(); k++)
		{
			if((*k).v0 == id) 
			{
				touch.first = FRONT;
				touch.second = k;
			}
		}
		for(std::list<FrontEdge>::iterator k = this->deads.begin(); k != this->deads.end(); k++)
		{
			if((*k).v0 == id)
			{
				touch.first = DEADS;
				touch.second = k; 
			}
		}

		//mark vertices close to candidate
		Mark(candidate);
		return id;
	}

	/* returns the sphere touching p0, p1, p2 of radius r such that
	the normal of the face points toward the center of the sphere */

	bool FindSphere(Vector3 &p0, Vector3 &p1, Vector3 &p2, Vector3 &center) {
		//we want p0 to be always the smallest one.
		Vector3 p[3];

		if(p0 < p1 && p0 < p2) {
			p[0] = p0;
			p[1] = p1;
			p[2] = p2;          
		} else if(p1 < p0 && p1 < p2) {
			p[0] = p1;
			p[1] = p2;
			p[2] = p0;
		} else {
			p[0] = p2;
			p[1] = p0;
			p[2] = p1;
		}
		Vector3 q1 = p[1] - p[0];
		Vector3 q2 = p[2] - p[0];  

		Vector3 up = cross(q1,q2);
		Scalar uplen = up.norm();

		//the three points are aligned
		if(uplen < 0.001*q1.norm()*q2.norm()) {
			return false;
		}
		up /= uplen;


		Scalar a11 = dot(q1,q1);
		Scalar a12 = dot(q1,q2);
		Scalar a22 = dot(q2,q2);

		Scalar m = 4*(a11*a22 - a12*a12);
		Scalar l1 = 2*(a11*a22 - a22*a12)/m;
		Scalar l2 = 2*(a11*a22 - a12*a11)/m;

		center = q1*l1 + q2*l2;
		Scalar circle_r = center.norm();
		if(circle_r > radius) {
			return false; //need too big a sphere
		}

		Scalar height = sqrt(radius*radius - circle_r*circle_r);
		center += p[0] + up*height;

		return true;
	}         

	/* compute angle from p to q, using axis for orientation */
	Scalar Angle(Vector3 p, Vector3 q, Vector3 &axis) {
		p.normalize();
		q.normalize();
		Vector3 vec = cross(p,q);
		Scalar angle = acos(dot(p,q));
		if(dot(vec,axis) < 0) angle = -angle;
		if(angle < 0) angle += 2*M_PI;
		return angle;
	}          

	void Mark(Vertex v) {
		KDResults matches;
		int n = tree->ball_search( points[v], min_edge, matches);
		for(int t = 0; t < n; t++) 
			used[ Vertex(matches[t].first) ] = true;
		visited[v] = true;
	}


	bool CheckEdge(int v0, int v1) {
		int tot = 0;
		foreach(SimpleFace f, fV)
		{
			for(int k = 0; k < 3; k++)
			{
				if(v0 == f.V0(k) && v1 == f.V1(k))  //orientation non consistent
					return false;              
				else if(v1 == f.V0(k) && v0 == f.V1(k)) 
					++tot;
			}
			if(tot >= 2) { //non manifold
				return false;
			}
		}
		return true;
	}        

	/// Front management:
	//Add a new FrontEdge to the back of the queue
	std::list<FrontEdge>::iterator NewEdge(FrontEdge e) {                  
		return front.insert(front.end(), e);
	}     

	//move an Edge among the dead ones
	void KillEdge(std::list<FrontEdge>::iterator e) 
	{
		if (e->active)
		{
			(*e).active = false;
			//std::list<FrontEdge>::iterator res = std::find(front.begin(),front.end(),e);
			FrontEdge tmp = *e;
			deads.splice(deads.end(), front, e);
			std::list<FrontEdge>::iterator newe = std::find(deads.begin(),deads.end(),tmp);
			tmp.previous->next = newe;
			tmp.next->previous = newe;
		}

	}

	void Erase(std::list<FrontEdge>::iterator e) {
		if((*e).active) front.erase(e);
		else deads.erase(e);
	}

	//move an FrontEdge to the back of the queue
	void MoveBack(std::list<FrontEdge>::iterator e) {
		front.splice(front.end(), front, e);          
	}

	void MoveFront(std::list<FrontEdge>::iterator e) {
		front.splice(front.begin(), front, e);
	}

	//check if e can be sewed with one of its neighbors
	bool Glue(std::list<FrontEdge>::iterator e) {
		return Glue((*e).previous, e) || Glue(e, (*e).next);
	}

	//Glue together a and b (where a.next = b
	bool Glue(std::list<FrontEdge>::iterator a, std::list<FrontEdge>::iterator b) {
		if((*a).v0 != (*b).v1) return false; 

		std::list<FrontEdge>::iterator previous = (*a).previous;
		std::list<FrontEdge>::iterator next = (*b).next;
		(*previous).next = next;
		(*next).previous = previous;
		Detach((*a).v1);
		Detach((*a).v0); 
		Erase(a);
		Erase(b);  
		return true;
	}

	void Detach(int v) {
		assert(nb[v] > 0);
		if(--nb[v] == 0) {
			border[Vertex(v)] = false;
		}
	}   

	double inline uniform(double a = 0.0, double b = 1.0){
		double len = b - a;
		return ((double)rand()/RAND_MAX) * len + a;
	}
};
