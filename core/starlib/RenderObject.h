#pragma once
#include <QColor>
#include <QGLWidget>
#include "Eigen/Dense"

class RenderObject{
    typedef Eigen::Vector3d Vector3;

public:
    
    class Base{
    protected:
        float _size;
        QColor _color;
    public:
        Base(float size, QColor color) : _size(size),_color(color){}
        virtual ~Base(){}
        virtual void draw(QGLWidget& widget)=0;
        Base& size(float size){ this->_size=size; return *this; }
        Base& color(QColor color){ this->_color=color; return *this; }
    };
    
    class Triangle : public Base{
    public:
        Vector3 p1,p2,p3;
        Triangle(Vector3 p1, Vector3 p2, Vector3 p3, QColor color=Qt::red);
        virtual void draw(QGLWidget& widget);
    };

    class Segment : public Base{
    public:
        Vector3 p1,p2;
        Segment(Vector3 p1, Vector3 p2, float size, QColor color=Qt::red);
        virtual void draw(QGLWidget& widget);
    };

    class Ray : public Base{
    public:
        Vector3 orig;
        Vector3 dir;
        float _scale;
        
        Ray(Vector3 orig, Vector3 dir, float size, QColor color=Qt::red, float scale=1);
        virtual void draw(QGLWidget& widget);
        Ray& scale(float scale){ this->_scale=scale; return *this; }
        
    };
    
    class Point : public Base{
    public:
        Vector3 p;
        Point(Vector3 p, float size, QColor color=Qt::red);
        virtual void draw(QGLWidget& widget);
    };

    class Text : public Base{
    public:
        int _x, _y;
        QString _text;
        Text(int x, int y, const QString& text, float size, QColor color=Qt::red);
        virtual void draw(QGLWidget& widget);
    };
};
