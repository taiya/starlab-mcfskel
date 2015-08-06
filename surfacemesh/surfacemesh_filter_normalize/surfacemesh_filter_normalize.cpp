#include "surfacemesh_filter_normalize.h"
#include "StarlabDrawArea.h"

using namespace SurfaceMesh;

QString printBounding(const Eigen::AlignedBox3d& box){
    QString retval;
    QTextStream sout(&retval);
    Vector3 c = box.center();
    Vector3 s = box.diagonal();
    sout << "Center[" << c.x() << " " << c.y() << " " << c.z() << "]" 
         << "  Size[" << s.x() << " " << s.y() << " " << s.z() << "]";
    return retval;
}

void surfacemesh_filter_normalize::applyFilter(RichParameterSet*){
    qDebug() << "Old bounding box: " << printBounding(mesh()->bbox());
    
    /// Just to be sure... update it
    mesh()->updateBoundingBox();        
    Eigen::AlignedBox3d bbox = mesh()->bbox();
    Vector3 offset = bbox.center();
    
    /// Normalize to have longest side size = 1
    Vector3 s = bbox.diagonal();
    Scalar scale = qMax(s.x(),qMax(s.y(),s.z()));
 
    Vector3VertexProperty points = mesh()->vertex_coordinates();
    foreach(Vertex v, mesh()->vertices()){
        Point& p = points[v];
        p.x() -= offset.x();
        p.y() -= offset.y();
        p.z() -= offset.z();
        p.x() /= scale;
        p.y() /= scale;
        p.z() /= scale;
    }
    
    /// And update it after, so we can reset the viewpoint
    mesh()->updateBoundingBox();
    if(drawArea()) drawArea()->resetViewport();
    
    qDebug() << "New bounding box: " << printBounding(mesh()->bbox());
}

Q_EXPORT_PLUGIN(surfacemesh_filter_normalize)


