#include <QTreeWidgetItem>

#include "SurfaceMeshModel.h"
#include "helpers/SurfaceMeshQForEachHelpers.h"

SurfaceMeshModel::SurfaceMeshModel(QString path, QString name) : Model(path, name){
    /// Allocate rendering system
    this->color = Qt::darkGray;
}

void SurfaceMeshModel::decorateLayersWidgedItem(QTreeWidgetItem* parent){
    /// Show face count on layer
    {
        QTreeWidgetItem *fileItem = new QTreeWidgetItem();
        fileItem->setText(1, "Vertices");    
        fileItem->setText(2, QString::number( n_vertices() ));
        parent->addChild(fileItem);
    }
    /// Show face count on layer
    {
        QTreeWidgetItem *fileItem = new QTreeWidgetItem();
        fileItem->setText(1, "Faces");    
        fileItem->setText(2, QString::number( n_faces() ));
        parent->addChild(fileItem);
    }
    /// Show path`
    {
        QTreeWidgetItem *fileItem = new QTreeWidgetItem();
        fileItem->setText(1, "Path");
        fileItem->setText(2, this->path);
        parent->addChild(fileItem);        
    }
}

void SurfaceMeshModel::updateBoundingBox(){
    Surface_mesh::Vertex_property<Point> points = this->get_vertex_property<Point>("v:point");
    Surface_mesh::Vertex_iterator vit, vend = this->vertices_end();    
    _bbox.setToNull();
    for (vit = this->vertices_begin(); vit != vend; ++vit)
        _bbox.unite( points[vit] );
}

SurfaceMeshForEachHalfedgeHelper SurfaceMeshModel::halfedges(){
    return SurfaceMeshForEachHalfedgeHelper(this);
}

SurfaceMeshForEachVertexHelper SurfaceMeshModel::vertices(){
    return SurfaceMeshForEachVertexHelper(this);
}

SurfaceMeshForEachEdgeHelper SurfaceMeshModel::edges(){
    return SurfaceMeshForEachEdgeHelper(this);
}

SurfaceMeshForEachFaceHelper SurfaceMeshModel::faces(){
    return SurfaceMeshForEachFaceHelper(this);
}

SurfaceMeshForEachOneRingEdgesHelper SurfaceMeshModel::onering_hedges(Surface_mesh::Vertex v){
    return SurfaceMeshForEachOneRingEdgesHelper(this,v);
}

void SurfaceMeshModel::remove_vertex(Vertex v){
	this->vdeleted_[v] = true;
	this->garbage_ = true;
}

QDebug operator<< (QDebug d, const Surface_mesh::Edge& edge) {
    d.nospace() << "Edge[" << edge.idx() << "]";
    return d.space();
}
