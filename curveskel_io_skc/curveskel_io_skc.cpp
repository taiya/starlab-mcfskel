#include "curveskel_io_skc.h"
#include <fstream>
#include "Document.h"
#include "CurveskelHelper.h"

using namespace std;
using namespace CurveskelTypes;

Starlab::Model* curveskel_io_skc::open(QString path){
    QString name = pathToName(path);
    CurveskelModel* model = new CurveskelModel(path,name);
    
    string inputLine;
    ifstream file( qPrintable(path) );
    float x,y,z;
    int n1, n2;
    int n_vertices;
    int n_edges;

    /// Check file
    if(!file.is_open())
        throw StarlabException("Cannot open skeleton file");

    /// Read header
    if(!file.eof() && getline(file, inputLine)) {
        if(sscanf(inputLine.c_str(), "%d %d", &n_vertices, &n_edges) != 2) {
            throw StarlabException("Error reading skeleton file (check header)");
            // printf("\n%d - %d - %d\n", degree, numberOfNodes, numberOfEdges);
        }
    }

    int i_vertices=0,i_edges=0;
    int status=0;
    /// Parse file
    while (!file.eof()) {
        getline(file, inputLine);
        
        /// Scanning vertices
        if(i_vertices<n_vertices){
            // qDebug() << "parsing: "<< inputLine.c_str();
            if(sscanf(inputLine.c_str(), "%f %f %f", &x, &y, &z) != 3)
                throw StarlabException("Invalid vertex found");
            model->add_vertex(Vector3(x,y,z));
            i_vertices++;
            continue;

        /// Scanning edges
        } else if(i_edges<n_edges) {
            status++;
            status = status%5;
            if(status==1){
                // qDebug() << "parsing[" <<status<<"]"<< inputLine.c_str();                
                if( sscanf(inputLine.c_str(), "%d %d", &n1, &n2) != 2)
                    throw StarlabException("Invalid edge found");
                model->add_edge(Vertex(n1),Vertex(n2));
                i_edges++;
            }
        }
    }
    file.close();

    // DEB qDebug("Parsed %d/%d vertices",i_vertices,n_vertices);
    // DEB qDebug("Parsed %d/%d edges",i_edges,n_edges);
    
    /// Show warnings
    if(i_vertices!=n_vertices) qWarning("Expected %d vertices, but parsed %d",n_vertices,i_vertices );
    if(i_edges!=n_edges) qWarning("Expected %d edges, but parsed %d",n_edges,i_edges );
         
#if 0
    /// Output Statistics
    model->property_stats();
    Vector3VertexProperty vcoord = model->get_vertex_property<Vector3>("v:point");
    foreach(Vertex v, model->vertices()){
        qDebug() << vcoord[v];
    }
#endif
    
    return model;
}

void curveskel_io_skc::save(CurveskelModel*, QString)
{
    throw StarlabException("Saving to 'skc' files is not implemented.");
}

Q_EXPORT_PLUGIN(curveskel_io_skc)
