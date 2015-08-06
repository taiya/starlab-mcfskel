#include "plugin.h"
#include "SurfaceMeshNormalsHelper.h"

using namespace SurfaceMesh;

bool modded_read_obj(SurfaceMeshModel& mesh, const std::string& filename){
    char   s[200];
    float  x, y, z;
    float  nx,ny,nz;
    std::vector<Surface_mesh::Vertex>  vertices;

    // open file (in ASCII mode)
    FILE* in = fopen(filename.c_str(), "r");
    if (!in) return false;

    // if mesh is not empty we need an offset for vertex indices
    // also take into accout that OBJ indices start at 1 (not 0)
    const int voffset = mesh.n_vertices() - 1;

    // clear line once
    memset(&s, 0, 200);
    
    bool hasnormals = false;
    
    // parse line by line (currently only supports vertex positions & faces
    while(in && !feof(in) && fgets(s, 200, in)) {
        // comment
        if (s[0] == '#' || isspace(s[0])) continue;

        // vertex
        else if (strncmp(s, "v ", 2) == 0) {
            if (sscanf(s, "v %f %f %f", &x, &y, &z)) {
                mesh.add_vertex(Surface_mesh::Point(x,y,z));
            }
        }
        
        // face
        else if (strncmp(s, "f ", 2) == 0) {
            int component(0), nV(0);
            bool endOfVertex(false);
            char *p0, *p1(s+1);

            vertices.clear();

            // skip white-spaces
            while (*p1==' ') ++p1;

            while (p1) {
                p0 = p1;

                // overwrite next separator

                // skip '/', '\n', ' ', '\0', '\r' <-- don't forget Windows
                while (*p1!='/' && *p1!='\r' && *p1!='\n' && *p1!=' ' && *p1!='\0') ++p1;

                // detect end of vertex
                if (*p1 != '/') {
                    endOfVertex = true;
                }

                // replace separator by '\0'
                if (*p1 != '\0') {
                    *p1 = '\0';
                    p1++; // point to next token
                }

                // detect end of line and break
                if (*p1 == '\0' || *p1 == '\n') {
                    p1 = 0;
                }

                // read next vertex component
                if (*p0 != '\0') {
                    switch (component) {
                    case 0: // vertex
                        vertices.push_back( Surface_mesh::Vertex(atoi(p0) + voffset) );
                        break;

                    case 1: // texture coord
                        break;

                    case 2: // normal
                        break;
                    }
                }

                ++component;

                if (endOfVertex) {
                    component = 0;
                    nV++;
                    endOfVertex = false;
                }
            }

            mesh.add_face(vertices);
        }
        else if( !hasnormals && (strncmp(s, "vn ", 3) == 0)){
            hasnormals = true;
        }


        // clear line
        memset(&s, 0, 200);
    }
    
    /// Normals
    if(hasnormals){
        /// Go back to beginning of file
        rewind(in);
        /// And start reading
        unsigned int ncounter = 0;
        Vector3VertexProperty normals = mesh.vertex_normals(true);
        while(in && !feof(in) && fgets(s, 200, in)) {   
            if (strncmp(s, "vn ", 3) == 0) {
                int nread = sscanf(s, "vn %f %f %f", &nx, &ny, &nz);
                assert(nread==3);
				if(ncounter >= mesh.n_vertices()) continue; // skip duplicated normals
                normals[Vertex(ncounter)] = Vector3(nx,ny,nz);
                // qDebug() << normals[Vertex(ncounter)];                
                ncounter++;    
            }
        }        
        
        if(ncounter!=mesh.n_vertices())
            qWarning("Warning: while reading file #normals=%d while #vertices=%d", (int)ncounter, mesh.n_vertices());  
    }
    
    fclose(in);
    return true;
}

Model* surfacemesh_io_obj::open(QString path) {
    SurfaceMeshModel* mesh = new SurfaceMeshModel(path);
    modded_read_obj(*mesh, qPrintable(path)); ///< ~copy/paste from Surface_mesh
    
    /// If they are not loaded from file, compute normals
    NormalsHelper h(mesh);
    if(!mesh->has_face_normals())
        h.compute_face_normals();
    if(!mesh->has_vertex_normals()) 
        h.compute_vertex_normals();        
        
    return mesh;
}

void surfacemesh_io_obj::save(SurfaceMeshModel* mesh,QString path) {
    FILE* fid = fopen( qPrintable(path), "w" );
    if( fid == NULL ) throw StarlabException("the file cannot be opened");

    Vector3VertexProperty p = mesh->vertex_coordinates();
    Vector3VertexProperty n = mesh->vertex_normals();

    foreach(Vertex v, mesh->vertices()){
        if(n) fprintf( fid, "vn %.10f %.10f %.10f\n", n[v].x(), n[v].y(), n[v].z() );        
        fprintf( fid, "v %.10f %.10f %.10f\n", p[v].x(), p[v].y(), p[v].z() );
    }

    foreach(Face f, mesh->faces()) {
        // int nV = mesh->valence(f);
        fprintf(fid, "f");
        foreach(Vertex v, mesh->vertices(f)){
            int ni = ((Surface_mesh::Vertex)v).idx()+1;
            if(n) fprintf(fid, " %d//%d", ni, ni);
            else  fprintf(fid, " %d", ni);
        }
        fprintf(fid,"\n");
    }

    fclose(fid);
}

Q_EXPORT_PLUGIN(surfacemesh_io_obj)
