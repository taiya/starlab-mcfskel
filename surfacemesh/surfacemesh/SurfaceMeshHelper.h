#pragma once
#include "StarlabException.h"
#include "SurfaceMeshModel.h"

/// Simplified access to property types and store names of defined properties
namespace SurfaceMesh{

/// @note when inheriting from this class, always use "class Subclass : public virtual SurfaceMeshHelper{}"
class SurfaceMeshHelper{
protected:
    SurfaceMesh::Model* mesh;
    Vector3VertexProperty points;
    ScalarVertexProperty  varea;  /// NULL
    Vector3VertexProperty vnormal; /// NULL
    Vector3FaceProperty   fnormal; /// NULL
	ScalarFaceProperty   farea; /// NULL
    ScalarEdgeProperty    elenght;  /// NULL
    
public:
    SurfaceMeshHelper(SurfaceMesh::Model* mesh) : mesh(mesh){
        points = mesh->vertex_property<Point>(VPOINT);
        vnormal = mesh->get_vertex_property<Vector3>(VNORMAL);
        varea = mesh->get_vertex_property<Scalar>(VAREA);
        fnormal = mesh->get_face_property<Vector3>(FNORMAL);  
		farea = mesh->get_face_property<Scalar>(FAREA);  
        elenght = mesh->get_edge_property<Scalar>(ELENGTH);
    }
    
    ScalarVertexProperty scalarVertexProperty(const std::string property, Scalar init){
        return mesh->vertex_property<Scalar>(property,init);
    }
    ScalarVertexProperty getScalarVertexProperty(const std::string property){
        ScalarVertexProperty prop = mesh->get_vertex_property<Scalar>(property);
        if(!prop) throw MissingPropertyException(property+" of type Scalar");
        return prop;
    }
    ScalarHalfedgeProperty getScalarHalfedgeProperty(const std::string property){
        ScalarHalfedgeProperty prop = mesh->get_halfedge_property<Scalar>(property);
        if(!prop) throw MissingPropertyException(property+" of type ScalarHEdgeProperty");
        return prop;
    }

    Vector3VertexProperty getVector3VertexProperty(const std::string property){
        Vector3VertexProperty prop = mesh->get_vertex_property<Vector3>(property);
        if(!prop) throw MissingPropertyException(property);
        return prop;
    }
    Vector3VertexProperty defaultedVector3VertexProperty(const std::string property, Vector3 init){
        return mesh->vertex_property<Vector3>(property,init);
    }
    
    Vector3FaceProperty vector3FaceProperty(const std::string property, Vector3 init){
        return mesh->face_property<Vector3>(property,init);
    }
    
	Surface_mesh::Face_property<Scalar> computeFaceAreas(std::string property=FAREA){
		farea = mesh->face_property<Scalar>(property);

		Surface_mesh::Face_iterator fit, fend=mesh->faces_end();
		Surface_mesh::Vertex_around_face_circulator vit, vend;
		QVector<Vector3> pnts;

		for (fit=mesh->faces_begin(); fit!=fend; ++fit){
			// Collect points of face
			pnts.clear(); vit = vend = mesh->vertices(fit);
			do{ pnts.push_back(points[vit]); } while(++vit != vend);

            farea[fit] = 0.5 * (pnts[1] - pnts[0]).cross(pnts[2] - pnts[0]).norm();
		}

		return farea;
	}
    
    ScalarEdgeProperty computeEdgeLengths(std::string property=ELENGTH){
        elenght = mesh->edge_property<Scalar>(property,0.0f);
        foreach(Edge eit,mesh->edges())
            elenght[eit] = mesh->edge_length(eit);
        return elenght;
    }


    ScalarVertexProperty computeVertexBarycentricArea(const std::string property=VAREA){
        varea = mesh->vertex_property<Scalar>(property);                
        // Scalar a;
        // Vertex v0,v1,v2;
        foreach(Vertex v, mesh->vertices())
            varea[v] = 0.0;
        throw StarlabException("something");        
        return varea;
    }

    ScalarVertexProperty computeVertexVoronoiArea(const std::string property=VAREA){
        varea = mesh->vertex_property<Scalar>(property);                
        Scalar a;
        Vertex v0,v1,v2;
        foreach(Vertex v, mesh->vertices())
            varea[v] = 0.0;
        foreach(Face f, mesh->faces()){
            Surface_mesh::Vertex_around_face_circulator vfit = mesh->vertices(f);
            v0 = vfit;
            v1 = ++vfit;
            v2 = ++vfit;
            
            // compute area
            a = 0.5 * (points[v1]-points[v0]).cross(points[v2]-points[v0]).norm();
            
            // distribute area to vertices
            varea[v0] += a/3.0;
            varea[v1] += a/3.0;
            varea[v2] += a/3.0;
        }
        return varea;
    }
    
public:
    class MissingPropertyException : public StarlabException{
    public:
        MissingPropertyException(const std::string& m) : StarlabException(m.c_str()){}
        // MissingPropertyException(const QString& m) : StarlabException(m){}
        QString type(){ return "Missing Property"; }
    };
};

}
