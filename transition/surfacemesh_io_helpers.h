#pragma once
#include "Document.h"
#include "SurfaceMeshHelper.h"

bool attempt_read_as_surfacemesh(QString path, SurfaceMeshModel* mesh){
    // qDebug("attempt_read_as_surfacemesh(%s,%s)",qPrintable(path),"mesh");
    return mesh->read(qPrintable(path));
}
bool attempt_read_as_medial_mesh(QString path, SurfaceMeshModel* mesh){
    // qDebug() << "attempt_read_as_medial_mesh()";
    
    int err = 0;
    unsigned int       i, j, idx;
    unsigned int       nV, nF, nE;
    Vec3f              p;
    float              alpha;
    float              radii;
    // Surface_mesh::Vertex  v;
    char               line[100], *c;
    bool               has_texcoords = false;
    bool               has_normals   = false;
    bool               has_colors    = false;
    bool               has_hcoords   = false;
    bool               has_dim       = false;
    bool               is_binary     = false;
    
    Q_ASSERT(mesh->n_vertices()==0);
    
    // open file (in ASCII mode)
    FILE* in = fopen(qPrintable(path), "r");
    if (!in) return false;
    
    // read comment
    c = fgets(line, 100, in);
    if (strncmp(c, "# MEDIAL", 8) != 0) { fclose(in); return false; } // no "# MEDIAL"
        
    // read header: [ST][C][N][4][n]OFF BINARY
    c = fgets(line, 100, in);
    assert(c != NULL);
    c = line;
    if (c[0] == 'S' && c[1] == 'T') { has_texcoords = true; c += 2; }
    if (c[0] == 'C') { has_colors  = true; ++c; }
    if (c[0] == 'N') { has_normals = true; ++c; }
    if (c[0] == '4') { has_hcoords = true; ++c; }
    if (c[0] == 'n') { has_dim     = true; ++c; }
    if (strncmp(c, "OFF", 3) != 0) { fclose(in); return false; } // no OFF
    if (strncmp(c+4, "BINARY", 6) == 0) is_binary = true;
    
    // colors, homogeneous coords, and vertex dimension != 3 are not supported
    if( !has_dim || has_colors || has_hcoords || is_binary || has_texcoords ){
        fclose(in);
        return false;
    }

    /// check 5-point dimensionality
    c = fgets(line, 100, in);
    if (strncmp(c, "5", 1) != 0) { 
        qDebug() << "expected: '5', received: " << c;
        fclose(in); return false; 
    }

    // #Vertice, #Faces, #Edges
    err = fscanf(in, "%d %d %d", (int*)&nV, (int*)&nF, (int*)&nE);
    mesh->reserve(nV, std::max(3*nV, nE), nF);
    
    // Allocate properties
    ScalarVertexProperty vangle = mesh->add_vertex_property<Scalar>("v:angle",0);
    ScalarVertexProperty vradii = mesh->add_vertex_property<Scalar>("v:radii",0);
    
    // read vertices: pos [norma] [texcoord]
    for (i=0; i<nV && !feof(in); ++i){
        err = fscanf(in, "%f %f %f %f %f", &p[0], &p[1], &p[2],&alpha,&radii);
        Vertex v = mesh->add_vertex(p);
        vangle[v] = alpha;
        vradii[v] = radii;
    }
    
    // read faces: #N v[1] v[2] ... v[n-1]
    std::vector<Surface_mesh::Vertex> vertices;
    for (i=0; i<nF; ++i){
        err = fscanf(in, "%d", (int*)&nV);
        vertices.resize(nV);
        /// ENSURES TRIANGULATION
        Q_ASSERT(nV==3); 
        for (j=0; j<nV; ++j){
            err = fscanf(in, "%d", (int*)&idx);
            vertices[j] = Surface_mesh::Vertex(idx);
        }
        mesh->add_face(vertices);
    }

    // File was successfully parsed.
    fclose(in);
    
    return true;
}

bool attempt_save_as_surfacemesh(QString path, const SurfaceMeshModel* mesh){
    qDebug() << "surfacemesh_io_off::attempt_save_as_surfacemesh()";
    
    if(mesh->garbage()){
        /// Save only works well with a garbage collected mesh... :(
        /// Make a deep copy, garbage collect it, then save it
        Surface_mesh copy(*mesh);
        copy.garbage_collection();
        copy.write( path.toStdString() );
    } else {
        mesh->write( path.toStdString());
    }
    return true;
}

bool attempt_save_as_medial_mesh(QString path, const SurfaceMeshModel* _mesh){
    qDebug() << "surfacemesh_io_off::attempt_save_as_medial_mesh()";
    
    /// To be able to use qforeach iterators
    SurfaceMeshModel* mesh = const_cast<SurfaceMeshModel*>(_mesh);
    
    bool isMedialMesh = mesh->property("MedialMesh").toBool();
    if(!isMedialMesh) return false;
    
    /// Must not contain garbage (pure indexes)
    Q_ASSERT(!mesh->garbage());  
    
    Vector3VertexProperty vpoint = mesh->get_vertex_property<Vector3>(VPOINT);
    ScalarVertexProperty  vangle = mesh->get_vertex_property<Scalar>("v:angle");
    ScalarVertexProperty  vradii = mesh->get_vertex_property<Scalar>("v:radii");

    if(!vangle) throw StarlabException("This is not a real medial mesh, missing v:angle");
    if(!vradii) throw StarlabException("This is not a real medial mesh, missing v:radii");
    
    FILE* out = fopen(qPrintable(path), "w");    
    
    /// Write header (see http://www.geomview.org/docs/html/OFF.html)
    fprintf(out, "# MEDIAL\n");
    fprintf(out, "nOFF\n");
    fprintf(out, "5\n"); // dimensionality of vector
    fprintf(out, "%d %d 0\n", mesh->n_vertices(), mesh->n_faces());
    
    /// Write vertices
    foreach(Vertex v, mesh->vertices()){
        const Vector3& p = vpoint[v];
        /// x y z alpha radii
        fprintf(out, "%.10f %.10f %.10f %.10f %.10f\n", p[0], p[1], p[2],vangle[v],vradii[v]);
    }
    
    /// Write faces
    foreach(Face f, mesh->faces()){
        int nV = mesh->valence(f);
        fprintf(out, "%d", nV);
        Surface_mesh::Vertex_around_face_circulator fvit=mesh->vertices(f), fvend=fvit;
        do{
            fprintf(out, " %d", ((Surface_mesh::Vertex)fvit).idx());
        } while (++fvit != fvend);
        fprintf(out, "\n");
    }
    
    fclose(out);
    /// It was indeed a medial mesh
    return true; 
}
