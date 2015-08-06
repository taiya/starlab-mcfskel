#pragma once

#include <cmath>
#include <set>
#include <stack>
#include <vector>
#include <list>
#include <stack>

#include <QSet>
#include <QtOpenGL>

#include "surface_mesh/Surface_mesh.h"
#include "primitives.h"

typedef std::set<int> IndexSet;
typedef IndexSet::iterator IndexSetIter;

// Ray-triangle acceleration
#include "TriAccel.h"

#define USE_TRI_ACCEL 1
#define DEFAULT_OCTREE_NODE_SIZE 40

class Octree
{
public:

	Octree(){ trianglePerNode = -1; parent = NULL; mesh = NULL; }

    Octree( Surface_mesh * useMesh, int triPerNode = DEFAULT_OCTREE_NODE_SIZE )
    {
        this->parent = NULL;
        this->mesh = useMesh;
        this->trianglePerNode = triPerNode;

        // Using all tris
        std::vector<Surface_mesh::Face> allTris;
        Surface_mesh::Face_iterator fit, fend = useMesh->faces_end();
        for(fit = useMesh->faces_begin(); fit != fend; ++fit)
            allTris.push_back(fit);

        this->initBuild(allTris, trianglePerNode);
    }

    Octree( int triPerNode, const BoundingBox& bb, const std::vector<Surface_mesh::Face>& tris, Surface_mesh * useMesh )
    {
        this->parent = NULL;
        this->mesh = useMesh;
        this->boundingBox = bb;
        this->trianglePerNode = triPerNode;
        this->triangleData = tris;
    }

    
    
    /// Uses eigen ray type
    Eigen::Vector3d closestIntersectionPoint( const Eigen::ParametrizedLine<double,3>& ray, int* faceIndex ){
        Ray _ray(ray.origin(), ray.direction());
        return closestIntersectionPoint(_ray, faceIndex);
    }
    
    Eigen::Vector3d closestIntersectionPoint( const Ray & ray, int * faceIndex )
    {
        HitResult res, best_res;
        Eigen::Vector3d isetpoint(0.0,0.0,0.0);
        double minDistance = DBL_MAX;
        if(faceIndex) *faceIndex = -1;

        // We use two tests, first is much faster but has misses.
        // The second is executed only if first fails

        // Fast, not robust tests
        foreach( int i, intersectRay( ray, 0, false ) )
        {
            intersectionTestAccelerated(SurfaceMesh::Model::Face(i), ray, res);

            // find the nearest intersection point
            if(res.hit)
            {
                if (res.distance < minDistance)
                {
                    minDistance = res.distance;
                    isetpoint = ray.origin + (ray.direction * res.distance);
                    if(faceIndex) *faceIndex = i;
                    best_res = res;
                }
            }
        }

        // Slower, more robust tests
        if(!best_res.hit){
            foreach( int i, intersectRay( ray, 0.01, false ) ){
                rayTriangleIntersectionTest(SurfaceMesh::Model::Face(i), ray, res, false);
                if(res.hit){
                    if (res.distance < minDistance){
                        minDistance = res.distance;
                        isetpoint = ray.origin + (ray.direction * res.distance);
                        if(faceIndex) *faceIndex = i;
                        best_res = res;
                    }
                }
            }
        }

        //assert(best_res.hit);

        return isetpoint;
    }

    IndexSet intersectSphere( const Eigen::Vector3d& sphere_center, double radius )
    {
        IndexSet tris;

        if (boundingBox.intersectsSphere(sphere_center, radius))
            intersectRecursiveSphere(sphere_center, radius, tris);

        return tris;
    }

	BoundingBox boundingBox;
	std::vector<Octree> children;
	std::vector<Surface_mesh::Face> triangleData;

private:
	int trianglePerNode;
	Surface_mesh * mesh;
    Surface_mesh::Vertex_property<Eigen::Vector3d> points;

    void initBuild( std::vector<Surface_mesh::Face>& tris, int triPerNode )
    {
        // add triangles involved to "triangleData"
        this->triangleData = tris;
        this->trianglePerNode = triPerNode;

        // Create a big box
        BoundingBox bb;

        // Collect faces geometry
        points = mesh->vertex_property<Eigen::Vector3d>("v:point");
        std::vector< std::vector<Eigen::Vector3d> > triangles ( mesh->n_faces() );
        Surface_mesh::Face_iterator fit, fend = mesh->faces_end();
        for(fit = mesh->faces_begin(); fit != fend; ++fit){
            std::vector<Eigen::Vector3d> pnts;
            Surface_mesh::Vertex_around_face_circulator vit = mesh->vertices(fit),vend=vit;
            do{ pnts.push_back(points[vit]); } while(++vit != vend);
            triangles[Surface_mesh::Face(fit).idx()] = pnts;
        }

        // Pre-compute per triangle quantities for fast ray-intersection
        #if USE_TRI_ACCEL
        Surface_mesh::Face_property< TriAcceld > tri_accel = mesh->face_property<TriAcceld>("f:TriAccel");
        for(fit = mesh->faces_begin(); fit != fend; ++fit){
            int i = Surface_mesh::Face(fit).idx();
            tri_accel[fit].load(triangles[i][0], triangles[i][1], triangles[i][2]);
        }
        #endif

        bb.computeFromTris(triangles);

        // Transform and scale to node's coordinates
        double largeSize = qMax(bb.xExtent, qMax(bb.yExtent, bb.zExtent));

        largeSize *= 1.25;

        // Define our bounding box
        this->boundingBox = BoundingBox(bb.center, largeSize, largeSize, largeSize);

        // Build the tree
        this->build();

        // Connect children with parent
        std::stack<Octree*> childStack;
        childStack.push(this);
        while(!childStack.empty())
        {
            Octree * curr = childStack.top(); childStack.pop();

            for(int i = 0; i < (int) curr->children.size(); i++)
            {
                curr->children[i].parent = curr;

                childStack.push( &curr->children[i] );
            }
        }
    }

    void newNode( int depth, double x, double y, double z )
    {
        double newExtent = boundingBox.xExtent / 2.0;

        Eigen::Vector3d center;

        center.x() = boundingBox.center.x() + (newExtent * x);
        center.y() = boundingBox.center.y() + (newExtent * y);
        center.z() = boundingBox.center.z() + (newExtent * z);

        BoundingBox bb(center, newExtent, newExtent, newExtent);

        // Add child
        children.push_back(Octree());
        Octree * child = &children.back();

        child->mesh = mesh;
        child->points = points;
        child->boundingBox = bb;
        child->trianglePerNode = this->trianglePerNode;

        // Collect triangles inside child's bounding box
        for(std::vector<Surface_mesh::Face>::iterator it = this->triangleData.begin(); it != this->triangleData.end(); it++)
        {
            Surface_mesh::Face face = *it;

            std::vector<Eigen::Vector3d> v = triPoints(face);

            if( bb.containsTriangle(v[0], v[1], v[2]) )
            {
                child->triangleData.push_back(face);
            }
        }

        child->build(depth + 1); // build it
    }

    std::vector<Surface_mesh::Face> getIntersectingTris(const Eigen::Vector3d& v0, const Eigen::Vector3d& v1, const Eigen::Vector3d& v2, bool showIt)
    {
        if(this->triangleData.size() == 0 || this->children.size() == 0)
            return this->triangleData;

        std::vector<Surface_mesh::Face> res;

        for(int i = 0; i < (int) this->children.size(); i++)
        {
            if(children[i].boundingBox.containsTriangle(v0, v1, v2))
            {
                const std::vector<Surface_mesh::Face> tris = children[i].getIntersectingTris(v0, v1, v2, showIt);

                for(int j = 0; j < (int) tris.size(); j++)
                    res.push_back(tris[j]);
            }
        }

        return res;
    }

    void build( int depth = 0 )
    {
        if ((int)triangleData.size() > this->trianglePerNode)
        {
            if(depth < 8)
            {
                // Subdivide to 8 nodes
                newNode(depth, -1, -1, -1);
                newNode(depth, 1, -1, -1);
                newNode(depth, -1, 1, -1);
                newNode(depth, 1, 1, -1);
                newNode(depth, -1, -1, 1);
                newNode(depth, 1, -1, 1);
                newNode(depth, -1, 1, 1);
                newNode(depth, 1, 1, 1);
            }
        }
    }

public:
    void DrawBox(const Eigen::Vector3d& center, float width, float length, float height, float r, float g, float b, float lineWidth)
    {
        Eigen::Vector3d  c1, c2, c3, c4;
        Eigen::Vector3d  bc1, bc2, bc3, bc4;

        c1 = Eigen::Vector3d (width, length, height) + center;
        c2 = Eigen::Vector3d (-width, length, height) + center;
        c3 = Eigen::Vector3d (-width, -length, height) + center;
        c4 = Eigen::Vector3d (width, -length, height) + center;

        bc1 = Eigen::Vector3d (width, length, -height) + center;
        bc2 = Eigen::Vector3d (-width, length, -height) + center;
        bc3 = Eigen::Vector3d (-width, -length, -height) + center;
        bc4 = Eigen::Vector3d (width, -length, -height) + center;

        glDisable(GL_LIGHTING);

        glColor3f(r, g, b);
        glLineWidth(lineWidth);

        glBegin(GL_LINES);
        glVertex3dv(c1.data());glVertex3dv(bc1.data());
        glVertex3dv(c2.data());glVertex3dv(bc2.data());
        glVertex3dv(c3.data());glVertex3dv(bc3.data());
        glVertex3dv(c4.data());glVertex3dv(bc4.data());
        glVertex3dv(c1.data());glVertex3dv(c2.data());
        glVertex3dv(c3.data());glVertex3dv(c4.data());
        glVertex3dv(c1.data());glVertex3dv(c4.data());
        glVertex3dv(c2.data());glVertex3dv(c3.data());
        glVertex3dv(bc1.data());glVertex3dv(bc2.data());
        glVertex3dv(bc3.data());glVertex3dv(bc4.data());
        glVertex3dv(bc1.data());glVertex3dv(bc4.data());
        glVertex3dv(bc2.data());glVertex3dv(bc3.data());
        glEnd();
        glEnable(GL_LIGHTING);
    }

    void draw( double r, double g, double b, double lineWidth )
    {
        if(root() == this)
        {
            foreach(Octree * iset, selectedChildren){
                //iset->draw(1,0,0,lineWidth + 3);
                BoundingBox bb = iset->boundingBox;
                DrawBox(bb.center, bb.xExtent, bb.yExtent, bb.zExtent,1,0,0, lineWidth + 2);
            }
        }

        DrawBox(boundingBox.center, boundingBox.xExtent, boundingBox.yExtent, boundingBox.zExtent,r,g,b, lineWidth);

        for (std::vector<Octree>::iterator child = children.begin();  child != children.end(); child++)
            child->draw(r,g,b, lineWidth);
    }

    IndexSet intersectPoint( const Eigen::Vector3d& point )
    {
        IndexSet tris;

        if (boundingBox.contains(point))
            intersectRecursivePoint(point, tris);

        return tris;
    }

    void intersectRecursivePoint( const Eigen::Vector3d& point, IndexSet& tris )
    {
        if (intersectHit(tris))
            return;

        for (std::vector<Octree>::iterator child = children.begin();  child != children.end(); child++)
        {
            if (child->boundingBox.contains(point))
                child->intersectRecursivePoint(point, tris);
        }
    }

    // Leaf node = true
    bool intersectHit( IndexSet& tris )
    {
        if( this->children.size() > 0 )
            return false;

        for(std::vector<Surface_mesh::Face>::iterator it = triangleData.begin(); it != triangleData.end(); it++)
        {
            Surface_mesh::Face face = *it;
            tris.insert( face.idx() );
        }

        // Debug:
        //root()->selectedChildren.push_back(this);

        // This is a leaf node
        return true;
    }

    QSet<int> intersectRay( Ray ray, double rayThickness, bool isFullTest) const
    {
        QSet<int> tris;

        ray.thickness = rayThickness;

        //DEBUG:
        //this->selectedChildren.clear();

        if ( this->boundingBox.intersects(ray) )
        {
            std::stack<const Octree*> s;
            s.push( this );

            while( !s.empty() )
            {
                const Octree * curTree = s.top();
                s.pop();

                if(curTree->children.size() == 0)
                {
                    for(std::vector<Surface_mesh::Face>::const_iterator it = curTree->triangleData.begin(); it != curTree->triangleData.end(); it++)
                    {
                        Surface_mesh::Face face = *it;
                        tris.insert( face.idx() );
                    }

                    // Debug:
                    //root()->selectedChildren.push_back(curTree);
                }

                // Do following if child size > 0
                for (std::vector<Octree>::const_iterator child = curTree->children.begin();  child != curTree->children.end(); child++)
                {
                    if ( child->boundingBox.intersects(ray) )
                    {
                        s.push( &(*child) );
                    }
                }
            }

            if(isFullTest)
            {
                QSet<int> exactSet;
                foreach(int i, tris)
                {
                    HitResult hitRes;
                    rayTriangleIntersectionTest(Surface_mesh::Face(i), ray, hitRes, false);
                    if(hitRes.hit) exactSet.insert(i);
                }
                return exactSet;
            }
        }

        return tris;
    }

    void intersectRecursiveRay( const Ray& ray, IndexSet& tris )
    {
        if(children.size() == 0)
        {
            for(std::vector<Surface_mesh::Face>::iterator it = triangleData.begin(); it != triangleData.end(); it++)
            {
                Surface_mesh::Face face = *it;
                tris.insert( face.idx() );
            }

            // Debug:
            //root()->selectedChildren.push_back(this);
        }

        // Do following if child size > 0
        for (std::vector<Octree>::iterator child = children.begin();  child != children.end(); child++)
        {
            if ( child->boundingBox.intersects(ray) )
            {
                child->intersectRecursiveRay(ray, tris);
            }
        }
    }

    void intersectRecursiveSphere( const Eigen::Vector3d& sphere_center, double radius, IndexSet& tris )
    {
        // Leaf node ?
        if (intersectHit(tris))
            return;

        // Visist children
        for (std::vector<Octree>::iterator child = children.begin();  child != children.end(); child++)
        {
            if (child->boundingBox.intersectsSphere(sphere_center, radius))
                child->intersectRecursiveSphere(sphere_center, radius, tris);
        }
    }


    bool testIntersectHit( const Ray& ray, HitResult & hitRes )
    {
        if(this->children.size() > 0)
            return false;

        // Do actual intersection test
        for(std::vector<Surface_mesh::Face>::iterator face = triangleData.begin(); face != triangleData.end(); face++)
        {
            Surface_mesh::Face f = *face;
            rayTriangleIntersectionTest(f, ray, hitRes, true);
            if(hitRes.hit)
                return true;
        }

        return false;
    }

    Octree * root()
    {
        if(parent == NULL)
            return this;
        else
            return parent->root();
    }

    std::vector<Eigen::Vector3d> triPoints(Surface_mesh::Face f) const
    {
        std::vector<Eigen::Vector3d> pnts;
        Surface_mesh::Vertex_around_face_circulator vit = mesh->vertices(f),vend=vit;
        do{ pnts.push_back(points[vit]); } while(++vit != vend);
        return pnts;
    }

    void rayTriangleIntersectionTest( Surface_mesh::Face f, const Ray & ray, HitResult & res, bool allowBack = true ) const
    {
        res.hit = false;
        res.distance = DBL_MAX;

        double EPS = 1e-7;

        std::vector<Eigen::Vector3d> v = triPoints(f);

        Eigen::Vector3d vertex1 = v[0];
        Eigen::Vector3d vertex2 = v[1];
        Eigen::Vector3d vertex3 = v[2];

        // Compute vectors along two edges of the triangle.
        Eigen::Vector3d edge1 = vertex2 - vertex1;
        Eigen::Vector3d edge2 = vertex3 - vertex1;

        // Compute the determinant.
        Eigen::Vector3d directionCrossEdge2 = cross(ray.direction, edge2);

        double determinant = dot(edge1, directionCrossEdge2);

        // If the ray is parallel to the triangle plane, there is no collision.
        if (fabs(determinant) < EPS)
            return;

        double inverseDeterminant = 1.0 / determinant;

        // Calculate the U parameter of the intersection point.
        Eigen::Vector3d distVector = ray.origin - vertex1;
        double triangleU = dot(distVector, directionCrossEdge2);
        triangleU *= inverseDeterminant;

        // Make sure it is inside the triangle.
        if (triangleU < 0 - EPS || triangleU > 1 + EPS)
            return;

        // Calculate the V parameter of the intersection point.
        Eigen::Vector3d distanceCrossEdge1 = cross(distVector, edge1);
        double triangleV = dot(ray.direction, distanceCrossEdge1);
        triangleV *= inverseDeterminant;

        // Make sure it is inside the triangle.
        if (triangleV < 0 - EPS || triangleU + triangleV > 1 + EPS)
            return;

        // Compute the distance along the ray to the triangle.
        double rayDistance = dot(edge2, distanceCrossEdge1);
        rayDistance *= inverseDeterminant;

        if(!allowBack){
            // Is the triangle behind the ray origin?
            if (rayDistance < 0)
                return;
        }

        res.hit = true;
        res.distance = rayDistance;

        res.u = triangleU;
        res.v = triangleV;

        res.index = f.idx();
    }

    void intersectionTestAccelerated( Surface_mesh::Face f, const Ray & ray, HitResult & res ) const
    {
        TriAcceld & triAccel = mesh->get_face_property<TriAcceld>("f:TriAccel")[f];
        double u, v, t;
        res.hit = triAccel.rayIntersect(TriAcceld::TriAccelRay(ray.origin, ray.direction), 0, DBL_MAX, u, v, t);

        res.distance = t;
        res.u = u;
        res.v = v;
    }


    Octree * parent;

	// Debug
	std::vector<Octree *> selectedChildren;
};

Q_DECLARE_METATYPE(Octree *)
