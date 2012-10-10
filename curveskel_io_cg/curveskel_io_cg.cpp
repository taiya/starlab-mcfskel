#include <QFile>
#include "curveskel_io_cg.h"
#include "Document.h"
#include "CurveskelHelper.h"
#include <fstream>

using namespace CurveskelTypes;

Model *curveskel_io_cg::open(QString path){
    DEB qDebug() << "curveskel_io_cg::open()";
    //assertValidPath(path);
    QString name = pathToName(path);
    CurveskelModel* model = new CurveskelModel(path,name);
    
    std::string inputLine;
    ifstream file( qPrintable(path) );
    float x,y,z;
    int n1, n2;
    int degree;
    int numberOfNodes;
    int numberOfEdges;
    
    /// Check file
    if(!file.is_open()) 
        throw StarlabException("Cannot open skeleton file");

    /// Read header
    if(!file.eof() && getline(file, inputLine)){
        if(sscanf(inputLine.c_str(), "# D:%d NV:%d NE:%d", &degree, &numberOfNodes, &numberOfEdges) != 3){
            qDebug("%d - %d - %d\n", degree, numberOfNodes, numberOfEdges);
            file.close();
            throw StarlabException("Error reading skeleton file");
        }
    }
    
    /// Parse file
    int nVertices = 0, nEdges = 0;
    while (!file.eof()){
        getline(file, inputLine);
		if(inputLine.length() < 1) continue;

        switch(inputLine[0]){
        case 'v':
            if(sscanf(inputLine.c_str(), "v %f %f %f", &x,&y,&z) == 3){
                model->add_vertex(Vector3(x,y,z));
                nVertices++;
            }                
            break;

        case 'e':
            if(sscanf(inputLine.c_str(), "e %d %d", &n1, &n2) == 2){
                model->add_edge(Vertex(n1-1),Vertex(n2-1));
                nEdges++;
            }
            break;
    }
    
    }    
    file.close();
    if(nVertices!=numberOfNodes) qDebug("Expected %d vertices, read %d",numberOfNodes,nVertices);
    if(nEdges!=numberOfEdges) qDebug("Expected %d vertices, read %d",numberOfEdges,nEdges);
        
    return model;
}

void curveskel_io_cg::save(CurveskelModel* skel, QString path)
{
    CurveskelModel::Vertex_property<CurveskelTypes::Point> pnts = skel->vertex_property<CurveskelTypes::Point>("v:point");
    skel->garbage_collection();

    QFile out(path);
    out.open(QIODevice::WriteOnly | QIODevice::Text);

    // Header
    out.write(qPrintable(QString("# D:3 NV:%1 NE:%2\n").arg(skel->n_vertices()).arg(skel->n_edges())));

    // Vertices
    foreach(CurveskelModel::Vertex v, skel->vertices())
        out.write(qPrintable(QString("v %1 %2 %3\n").arg(pnts[v].x()).arg(pnts[v].y()).arg(pnts[v].z())));

    // Edges (index from 1)
    foreach(CurveskelModel::Edge e, skel->edges())
        out.write(qPrintable(QString("e %1 %2\n").arg(1+skel->vertex(e,0).idx()).arg(1+skel->vertex(e,1).idx())));

    out.close();
}

Q_EXPORT_PLUGIN(curveskel_io_cg)

