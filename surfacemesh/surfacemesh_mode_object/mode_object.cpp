#include "mode_object.h"
#include <Eigen/Dense>
#include <Eigen/Geometry>

Q_EXPORT_PLUGIN(mode_object)

#include <limits>
#include <QKeyEvent>
#include "StarlabDrawArea.h"
#include "SurfaceMeshModel.h"

using namespace Eigen;

void mode_object::draw(){
    drawArea()->displayMessage(command, INT_MAX);
}

/// Defaults to "x" when you give it something weird
Vector3d char_to_vector3(char axisflag){
    Vector3d axis;
    switch(axisflag){
        case 'x': axis = Vector3d::UnitX(); break;
        case 'y': axis = Vector3d::UnitY(); break;
        case 'z': axis = Vector3d::UnitZ(); break;
        case 'a': axis = Vector3d::Ones(); break;
        default: axis = Vector3d::UnitX(); break;
    }
    return axis;
}

void mode_object::execute(QString command){
    /// @todo add a generic model.transform() to StarlabModel, then we won't depend on SurfaceMesh
    SurfaceMesh::Model* mesh = qobject_cast<SurfaceMesh::Model*>( selection() );
    if(!mesh){ 
        showMessage("Object mode only supports SurfaceMeshModel");    
        return;
    }
    
    /// Otherwise eigen::map fails
    mesh->garbage_collection();
    
    /// Map mesh to an eigen set
    Eigen::Map<Matrix3Xd> MESH((double *)(mesh->vertex_coordinates().data()), 3, mesh->n_vertices());

    /// r x 120
    if( command.startsWith("r") ){
        char axisflag='x';
        double angle=0;
        int nread = sscanf(command.toStdString().c_str(), "r %c %lf", &axisflag, &angle);
        if(nread != 2){ showMessage("Incorrect transformation :("); return; }
        Vector3d axis = char_to_vector3(axisflag);
        angle = angle * M_PI / 180;
        AngleAxisd rot( angle, axis );
        MESH = rot.toRotationMatrix()*MESH;
    }
    
    /// t x .1
    if( command.startsWith("t") ){       
        /// Parse
        char axisflag='x';
        double offset=0;
        int nread = sscanf(command.toStdString().c_str(), "t %c %lf", &axisflag, &offset);
        if(nread != 2){ showMessage("Incorrect transformation :("); return; }
        Vector3d axis = offset*char_to_vector3(axisflag);
        MESH.colwise() += axis;
    }

    /// s x .1
    if( command.startsWith("s") ){
        /// Parse
        char axisflag='a';
        double offset=0;
        int nread = sscanf(command.toStdString().c_str(), "s %c %lf", &axisflag, &offset);
        if(nread != 2){ showMessage("Incorrect transformation :("); return; }
        Vector3d axis = offset*char_to_vector3(axisflag);
        MESH = axis.asDiagonal()*MESH;
    }
    
    showMessage("Transformation applied!");    
}

bool mode_object::keyReleaseEvent(QKeyEvent* ke){
    //Qt::Key skip = Qt::Key_Backspace | Qt::Key_Escape;
    //if(ke->key().)
    //ke->key()
    
    /// @bug mode plugin is intercepting the escape command :(
    if( ke->key() == Qt::Key_Escape){
        command.clear();
        draw();
        return true;
    }
    if( ke->key() == Qt::Key_Backspace ){
        command.chop(1);
        draw();
        return true;
    }
    if( (ke->key() == Qt::Key_Return) || (ke->key() == Qt::Key_Enter) ){
        execute(command);
        command.clear();
        draw();
        return true;
    }
    
    /// Append text to buffer
    command.append(ke->text());       

    /// Auto-add whitespace
    if( ke->key() == Qt::Key_R || 
        ke->key() == Qt::Key_T || 
        ke->key() == Qt::Key_S ||
        ke->key() == Qt::Key_X || 
        ke->key() == Qt::Key_Y || 
        ke->key() == Qt::Key_Z ||
        ke->key() == Qt::Key_A)
        command.append(" ");
    
    /// Update message
    draw();
    
    /// Capture all events!!
    return true;
}
