#include "surfacemesh_io_off.h"

Model* surfacemesh_io_off::open(QString path){
    SurfaceMeshModel* mesh = new SurfaceMeshModel(path);
    bool has_loaded = mesh->read(qPrintable(path));
    if(has_loaded) return mesh;
    delete mesh;
    throw StarlabException("surfacemesh_io_off::open failed");
}   

void surfacemesh_io_off::save(SurfaceMeshModel* mesh,QString path){
    bool success;
    if(mesh->garbage()){
        /// Save only works well with a garbage collected mesh... ~_~
        /// Make a deep copy, garbage collect it, then save it
        /// As we might want to keep original!!!
        Surface_mesh copy(*mesh);
        copy.garbage_collection();
        success = copy.write( path.toStdString() );
    } else {
        success = mesh->write( path.toStdString());
    }
    
    if(!success) 
        throw StarlabException("surfacemesh_io_off::save failed");
}

Q_EXPORT_PLUGIN(surfacemesh_io_off)
