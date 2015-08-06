#include "filter_icp.h"
Q_EXPORT_PLUGIN(filter_icp)

//#include "helpers/MatlabSurfaceMeshHelper.h"
#include <QElapsedTimer>
#include <fstream>
#include <cfloat>
#include <limits>
#include <iomanip>
#include "NanoKDTree3.h"
#include "RigidMotionEstimator.h"
#include "StarlabDrawArea.h"

/// Decouple from this?
namespace SurfaceMesh{
    class VertexCoordinatesIterator : public std::iterator<std::forward_iterator_tag,Vector3>{
    private:
        Surface_mesh::Vertex_iterator it;
        typedef VertexCoordinatesIterator Iterator;
        Vector3VertexProperty points;
        Model* mesh;
    public:
        VertexCoordinatesIterator(Model* mesh){
            points = mesh->vertex_coordinates();
            it = mesh->vertices_begin();
            this->mesh = mesh;
        }
    public:
        bool operator!=(Iterator it){ return this->it != it.it; }
        Iterator&  operator++(){ ++it; return *this; } // ++myInstance.
        value_type  operator*(){
            Point& p = points[ Vertex(it) ];
            return Vector3(p.x(), p.y(), p.z());
        }
        VertexCoordinatesIterator begin(){  this->it=mesh->vertices_begin(); return *this; }
        VertexCoordinatesIterator end(){ this->it=mesh->vertices_end();   return *this; }
    };
} // namespace

const QString POINT_TO_PLANE = "Point to Plane";
const QString POINT_TO_POINT = "Point to Point";

void filter_icp::initParameters(RichParameterSet* pars){
    drawArea()->deleteAllRenderObjects();
    
    /// Avoids inserting selection 
    QStringList layers;
    foreach(StarlabModel* model, document()->models())
        if(SurfaceMesh::isA(model) && model!=mesh())
            layers << model->name;

    QStringList mode;
    mode << POINT_TO_PLANE;
    mode << POINT_TO_POINT;
    
    pars->addParam(new RichInt("it", 1, "ICP max iterations"));
    pars->addParam(new RichFloat("maxdist", 0.1, "max distance"));
    pars->addParam(new RichFloat("stop", 1e-5, "max norm"));
    pars->addParam(new RichBool("use_normalization", false, "Use normalization"));
    pars->addParam(new RichBool("draw_correspondences", true, "Draw correspondences"));
    pars->addParam(new RichStringSet("layer", layers, "Align with this mesh"));
    pars->addParam(new RichStringSet("mode", mode, "ICP mode"));
}

void filter_icp::applyFilter(RichParameterSet* pars) {
    std::cout << std::setprecision(std::numeric_limits<double>::digits10);
    QElapsedTimer timer;

    /// Fetch parameters
    int it = pars->getInt("it");
    double maxdist = pars->getFloat("maxdist");
    double stop = pars->getFloat("stop");
    bool use_normalization = pars->getBool("use_normalization");
    bool draw_correspondences = pars->getBool("draw_correspondences");
    QString layer = pars->getString("layer");
    QString mode = pars->getString("mode");
    bool is_point_to_plane = (mode == POINT_TO_PLANE);
    bool is_point_to_point = (mode == POINT_TO_POINT);
    
    
    //////////////////////////////////////////////////
    Model *m1 = SurfaceMesh::safe_cast(document()->selectedModel());
    Model *m2 = SurfaceMesh::safe_cast(document()->getModel(layer));
    m1->color = QColor(186, 172, 96);
    m2->color = QColor(86, 117, 147);
    
    //////////////////////////////////////////////////
    Eigen::Map<Eigen::Matrix3Xd> X((double *)(m1->vertex_coordinates().data()), 3, m1->n_vertices());
    Eigen::Map<Eigen::Matrix3Xd> dst((double *)(m2->vertex_coordinates().data()), 3, m2->n_vertices());
    Eigen::Map<Eigen::Matrix3Xd> dst_normals((double *)(m2->vertex_normals().data()), 3, m2->n_vertices());
    Eigen::Vector3d mean = dst.rowwise().sum()/static_cast<double>(m2->n_vertices());
    
    /// NORMALIZATION
    if(use_normalization) {
        X.colwise() -= mean;
        dst.colwise() -= mean;
        m2->updateBoundingBox();
        const QBox3D& bbox = m2->bbox();
        QVector3D s = bbox.size();
        double scale = qMax(s.x(),qMax(s.y(),s.z()));
        X/=scale;
        dst/=scale;
        m1->updateBoundingBox();
        m2->updateBoundingBox();
        drawArea()->resetViewport();
    }
    drawArea()->updateGL();
    QApplication::processEvents();
    
    
    //////////////////////////////////////////////////
    VertexCoordinatesIterator vci(m2);
    NanoKDTree3<Vector3> kdtree(vci.begin(), vci.end());
    
    //////////////////////////////////////////////////
    Eigen::Matrix3Xd Yp = Eigen::Matrix3Xd::Zero(3, m1->n_vertices());
    Eigen::Matrix3Xd Yn = Eigen::Matrix3Xd::Zero(3, m1->n_vertices());
    Eigen::Matrix3Xd src_old1 = X;
    
    //////////////////////////////////////////////////
    for(int icp=0; icp<it; showProgress(double(++icp)/it)) {
        /// CLOSEST POINT (UPDATES Yp, Yn)
        { 
            timer.restart();
            // #pragma omp parallel for
            for(int i=0; i<X.cols(); ++i) {
                int idx = kdtree.closest_index( X.col(i).data() );
                Eigen::Vector3d closest_pt = dst.col(idx);
                Eigen::Vector3d closest_nr = dst_normals.col(idx);
                Yp.col(i) = closest_pt;
                Yn.col(i) = closest_nr;
            }
            std::cout << "Closest points " << timer.elapsed() << std::endl;
        }

        /// DRAWS CORRESPONDENCES
        if (draw_correspondences) {
            drawArea()->deleteAllRenderObjects();
            for(unsigned int i=0; i<X.cols(); ++i) {
                Eigen::VectorXd r = (X-Yp).colwise().norm();
                QVector3D p1(X(0,i),  X(1,i), X(2,i));
                QVector3D p2(Yp(0,i),Yp(1,i),Yp(2,i));
                if(r(i) > maxdist)
                    drawArea()->drawSegment(p1, p2, 1, Qt::red);
                else
                    drawArea()->drawSegment(p1, p2, 1, Qt::green);
            }
        }   

        /// ROTATION ESTIMATION (UPDATES R,t)
        {
            timer.restart();
                
                /// Covariance entries weights
                Eigen::VectorXd weights = Eigen::VectorXd::Ones(X.cols());
    
                /// (SET WEIGHTS WHERE DIST>MAXDIST TO ZERO)
                weights = (X-Yp).colwise().norm();
                weights.array() /= std::min(maxdist, weights.maxCoeff()) + 1e-8;
                weights = weights.array().min(1.0);
                weights.array() *= -1.0;
                weights.array() += 1.0;
    
                /// Which version of ICP should we be using?
                if(is_point_to_point) 
                    RigidMotionEstimator::point_to_point(X, Yp, weights);
                if(is_point_to_plane) 
                    RigidMotionEstimator::point_to_plane(X, Yp, Yn, weights);
            
            std::cout << "Rotation, translation " << timer.elapsed() << std::endl;
        }

        /// stopping criteria
        {
            double final = (X-src_old1).colwise().norm().maxCoeff();
            src_old1 = X;
            std::cout << "final: " << final << std::endl;
            if(final < stop) 
                break;
        }       

        /// forcefully redraw
        {
            drawArea()->updateGL();
            QApplication::processEvents();
        }
    }
}
