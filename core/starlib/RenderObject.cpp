#include "RenderObject.h"
#include <QGLWidget>
#include <QDebug>

void RenderObject::Base::draw(QGLWidget& /*widget*/){
    glColor3f(_color.redF(), _color.greenF(), _color.blueF());   
}

RenderObject::Triangle::Triangle(Vector3 p1, Vector3 p2, Vector3 p3, QColor color) : Base(0.0f,color){
    this->p1 = p1;
    this->p2 = p2;
    this->p3 = p3;
    this->_color = color;
}

void RenderObject::Triangle::draw(QGLWidget& widget){
    Base::draw(widget);

    glDisable(GL_LIGHTING);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glBegin(GL_POLYGON); 
        glVertex3d(p1.x(),p1.y(),p1.z());
        glVertex3d(p2.x(),p2.y(),p2.z());
        glVertex3d(p3.x(),p3.y(),p3.z());
    glEnd();
}

RenderObject::Point::Point(Vector3 p, float size, QColor color) : Base(size,color){
    this->p = p;
}

void RenderObject::Point::draw(QGLWidget& widget){
    Base::draw(widget);
    
    // qDebug() << "RenderObject::Point::render()";
    glPointSize(_size);    
    glDisable(GL_LIGHTING);
    glBegin(GL_POINTS);
        glVertex3d(p.x(),p.y(),p.z());
    glEnd();
}

RenderObject::Segment::Segment(Vector3 p1, Vector3 p2, float size, QColor color):Base(size, color){
    this->p1 = p1;
    this->p2 = p2;
}

void RenderObject::Segment::draw(QGLWidget& widget){
    Base::draw(widget);

    glLineWidth(_size);
    glDisable(GL_LIGHTING);
    glBegin(GL_LINES);
        glVertex3d(p1.x(),p1.y(),p1.z());
        glVertex3d(p2.x(),p2.y(),p2.z());
    glEnd();
}

RenderObject::Ray::Ray(Vector3 orig, Vector3 dir, float size, QColor color, float scale):Base(size,color){
    this->orig=orig;
    this->dir=dir;
    this->_scale=scale;
}

void RenderObject::Ray::draw(QGLWidget& widget){
    Base::draw(widget);
    glLineWidth(_size);
    glDisable(GL_LIGHTING);
    Vector3& p1=orig;
    Vector3 p2=orig+_scale*dir;
    glBegin(GL_LINES);
        glVertex3d(p1.x(),p1.y(),p1.z());
        glVertex3d(p2.x(),p2.y(),p2.z());
    glEnd();
}


RenderObject::Text::Text(int x, int y, const QString& text, float size, QColor color) :
    Base(size,color),
    _x(x),
    _y(y),
    _text(text){}

void RenderObject::Text::draw(QGLWidget& widget){
    Base::draw(widget);
    widget.renderText(_x, _y, _text, QFont("Time New Roman", _size));
}
