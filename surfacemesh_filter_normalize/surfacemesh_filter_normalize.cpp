#include "surfacemesh_filter_normalize.h"
#include "StarlabDrawArea.h"
#include "SurfaceMeshTypes.h"
#include "SurfaceMeshHelper.h"

using namespace SurfaceMeshTypes;

QString printBounding(QBox3D box){
    QString retval;
    QTextStream sout(&retval);
    QVector3D c = box.center();
    QVector3D s   = box.size();
    sout << "Center[" << c.x() << " " << c.y() << " " << c.z() << "]" 
         << "  Size[" << s.x() << " " << s.y() << " " << s.z() << "]";
    return retval;
}

void surfacemesh_filter_normalize::applyFilter(RichParameterSet*){
    SurfaceMeshHelper helper( mesh() );
    
    qDebug() << "Old bounding box: " << printBounding(mesh()->bbox());
    
    /// Just to be sure... update it
    mesh()->updateBoundingBox();        
    QBox3D bbox = mesh()->bbox();
    Vector3 offset = bbox.center();
    
    /// Normalize to have longest side size = 1
    QVector3D s = bbox.size();
    Scalar scale = qMax(s.x(),qMax(s.y(),s.z()));
 
    Vector3VertexProperty points = helper.getVector3VertexProperty("v:point");
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


