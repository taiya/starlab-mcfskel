#include "SurfaceMeshHelper.h"
#include "qglviewer/manipulatedFrame.h"
using namespace qglviewer;
using namespace SurfaceMesh;

class ARAPDeformerHandle: public ManipulatedFrame{
    Q_OBJECT

public:
    ARAPDeformerHandle(const Vector3 & start, double Radius){
        this->startPos = start;
        this->radius = Radius;
        this->setPosition(start.x(), start.y(), start.z());
    }

    Vector3 transform(const Vector3 & originalPos){
        Vector3 d = originalPos - startPos;
        Vec delta(d[0], d[1], d[2]);
        Vec rotatedDelta = this->rotation() * delta;
        Vec r = this->position() + rotatedDelta;
        Vector3 newPos(r.x, r.y, r.z);

        double alpha = 1 - gaussianFunction(((originalPos - startPos).norm() / radius));

        return (originalPos * (alpha)) + (newPos * (1-alpha));
    }

private:
    Vector3 startPos;
    double radius;

    double inline gaussianFunction(double x, double mu = 0.0, double sigma = 2){
        //double a = 1.0 / (sigma * sqrt(2 * M_PI));
        double b = mu;
        double c = sigma;
        return exp( - (pow(x - b, 2) / (2 * pow(c, 2)) ) );
    }
};
