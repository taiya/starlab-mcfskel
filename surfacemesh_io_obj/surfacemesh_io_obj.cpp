#include "surfacemesh_io_obj.h"
#include "Document.h"
#include "SurfaceMeshHelper.h"

Model* surfacemesh_io_obj::open(QString path){
    typedef Surface_mesh::Vertex Vertex;
    char   s[200]; 
    memset(&s, 0, 200);
    float  x, y, z;
    int v1,v2,v3;
    float p1,p2,p3,radii,angle;
    int vertexCounter = 0;
    int faceCounter = 0;
    
    SurfaceMeshModel* mesh = new SurfaceMeshModel(path);
    
    // open file (in ASCII mode)
    FILE* in = fopen(qPrintable(path), "r");
    if (!in) throw StarlabException("Cannot open file");

    // OBJ indices start at 1 (not 0)
    // parse line by line (currently only supports vertex positions & faces)

    /// Create it just to be safe
    bool medialDataAllocated = false;
    int mVertexCounter=0;
    Vector3VertexProperty vpole;
    ScalarVertexProperty vradii;
    ScalarVertexProperty vangle;
        
    while(in && !feof(in) && fgets(s, 200, in)){
        // ignore comments   
        if (s[0] == '#' || isspace(s[0])){
            continue;
        } else 
        // vertex
        if (strncmp(s, "v ", 2) == 0){
            if (sscanf(s, "v %f %f %f", &x, &y, &z)){
                mesh->add_vertex(Point(x,y,z));
                vertexCounter++;
            }
        } else 
        // face
        if (strncmp(s, "f ", 2) == 0){
            if (sscanf(s, "f %d %d %d", &v1, &v2, &v3)){
                mesh->add_triangle(Vertex(v1-1), Vertex(v2-1), Vertex(v3-1));
                faceCounter++;
            }
        }
        // medial stuff
        if (strncmp(s, "m ", 2) == 0){
            if(!medialDataAllocated){
                vpole = mesh->vertex_property<Vector3>("v:pole",Vector3(0,0,0));
                vradii = mesh->vertex_property<Scalar>("v:radii",0.0);
                vangle = mesh->vertex_property<Scalar>("v:angle",0.0);
            }
            if( sscanf(s, "m %f %f %f %f %f", &p1, &p2, &p3, &radii, &angle) ){   
                Vertex v(mVertexCounter);
                vpole[v] = Vector3(p1,p2,p3);
                vradii[v] = radii;
                vangle[v] = angle;
                mVertexCounter++;
            }
        }
    }    
    
    /// post-process checks
    if(medialDataAllocated && (vertexCounter != mVertexCounter))
        throw StarlabException("#Poles != #Vertices");
    
    return mesh;
}

void surfacemesh_io_obj::save(SurfaceMeshModel *mesh, QString filename){
    FILE* out = fopen(qPrintable(filename), "w");
    if (!out) throw StarlabException("Cannot open file");

    // header
    fprintf(out, "# %d %d\n", mesh->n_vertices(), mesh->n_faces());   
    SurfaceMeshHelper h(mesh);
    
    // vertices
    Vector3VertexProperty points = h.getVector3VertexProperty(VPOINT);
    foreach(Vertex v, mesh->vertices()){
        const Point& p = points[v];
        fprintf(out, "v %.10f %.10f %.10f\n", p[0], p[1], p[2]);                
    }

    // faces
    foreach(Face f, mesh->faces()){
        /// @todo create iterator mesh->vertices(face) 
        fprintf(out, "f");
        Surface_mesh::Vertex_around_face_circulator fvit=mesh->vertices(f), fvend=fvit;
        do{
            fprintf(out, " %d", ((Surface_mesh::Vertex)fvit).idx()+1);
        } while (++fvit != fvend);
        fprintf(out, "\n");
    }
    
    // check whether it has all the medial properties
    bool p1 = mesh->has_vertex_property<Scalar>("v:radii");
    bool p2 = mesh->has_vertex_property<Scalar>("v:angle");
    bool p3 = mesh->has_vertex_property<Vector3>("v:pole");
    if(p1 && p2 && p3){
        ScalarVertexProperty vradii = h.getScalarVertexProperty("v:radii");
        ScalarVertexProperty vangle = h.getScalarVertexProperty("v:angle");
        Vector3VertexProperty vpole = h.getVector3VertexProperty("v:pole");
        foreach(Vertex v, mesh->vertices()){
            Vector3 p = vpole[v];
            fprintf(out, "m %.10f %.10f %.10f %10f %10f\n", p[0], p[1], p[2],vradii[v],vangle[v]);                
        }
    }
    
    fclose(out);
}
Q_EXPORT_PLUGIN(surfacemesh_io_obj)

