#pragma once

#include "RenderObject.h"
#include "Eigen/Dense"

#define glVertQt(v) glVertex3d(v.x(), v.y(), v.z())
#define glColorQt(c) glColor4d(c.redF(), c.greenF(), c.blueF(), c.alphaF())

class PointSoup : public RenderObject::Base{
    QVector< Vector3 > points;
	QVector< QColor > points_colors;
public:
	PointSoup():RenderObject::Base(1, Qt::black){}

    virtual void draw(QGLWidget& widget){
        Q_UNUSED(widget)

		glDisable(GL_LIGHTING);

		glPointSize(6);
		glBegin(GL_POINTS);
		for(int i = 0; i < (int) points.size(); i++){
			glColorQt(points_colors[i]);
			glVertQt(points[i]);
		}
		glEnd();

		glEnable(GL_LIGHTING);
	}

    void addPoint(const Vector3& p, const QColor& c = Qt::blue){
		points.push_back(p);
		points_colors.push_back(c);
	}
};

class VectorSoup : public RenderObject::Base{
    QVector< QPair<Vector3,Vector3> > vectors;
    QVector< double > vectorLengths;
    double maxLen;
public:
    VectorSoup(const QColor& c = Qt::green):RenderObject::Base(1, c){ maxLen = -DBL_MAX; }

    void addVector(const Vector3& start, const Vector3& direction){
        vectors.push_back( qMakePair(start,direction) );
        double l = direction.norm();
        vectorLengths.push_back(l);
        maxLen = qMax(l,maxLen);
    }

    virtual void draw(QGLWidget& widget){
        Q_UNUSED(widget)

        glDisable(GL_LIGHTING);
        glLineWidth(1);
        glBegin(GL_LINES);
        for(int i = 0; i < (int) vectors.size(); i++){
            // Color
            double d = vectorLengths[i] / maxLen;
            QColor c( _color.red() * d, _color.green() * d, _color.blue() * d );
            glColorQt(c);

            // Line
            glVertQt(vectors[i].first);
            glVertQt((vectors[i].first + vectors[i].second));
        }
        glEnd();

        glPointSize(3);
        glBegin(GL_POINTS);
        for(int i = 0; i < (int) vectors.size(); i++){
            // Color
            double d = vectorLengths[i] / maxLen;
            QColor c( _color.red() * d, _color.green() * d, _color.blue() * d );
            glColorQt(c);

            // Point
            glVertQt((vectors[i].first + vectors[i].second));
        }
        glEnd();
        glEnable(GL_LIGHTING);
    }
};

static QColor qtJetColorMap(double value, double min = 0.0, double max = 1.0)
{
    unsigned char rgb[3];
    unsigned char c1=144;
    float max4=(max-min)/4;
    value-=min;
    if(value==HUGE_VAL)
    {rgb[0]=rgb[1]=rgb[2]=255;}
    else if(value<0)
    {rgb[0]=rgb[1]=rgb[2]=0;}
    else if(value<max4)
    {rgb[0]=0;rgb[1]=0;rgb[2]=c1+(unsigned char)((255-c1)*value/max4);}
    else if(value<2*max4)
    {rgb[0]=0;rgb[1]=(unsigned char)(255*(value-max4)/max4);rgb[2]=255;}
    else if(value<3*max4)
    {rgb[0]=(unsigned char)(255*(value-2*max4)/max4);rgb[1]=255;rgb[2]=255-rgb[0];}
    else if(value<max)
    {rgb[0]=255;rgb[1]=(unsigned char)(255-255*(value-3*max4)/max4);rgb[2]=0;}
    else {rgb[0]=255;rgb[1]=rgb[2]=0;}
    return QColor(rgb[0],rgb[1],rgb[2]);
}
