#include "filter_depthscan.h"
Q_EXPORT_PLUGIN(filter_depthscan)

#include <QtOpenGL>
#include <qgl.h>
#include "SurfaceMeshModel.h"
#include "StarlabDrawArea.h"
#include "SurfaceMeshNormalsHelper.h"
#include "SurfaceMesh/DepthTriangulator.h"
#include "Eigen/Dense"
#include "Octree.h"

using namespace SurfaceMesh;
using namespace Eigen;

/// @brief generate a sample of a specific gaussian distribution using the Box-Muller transform.
///        See http://en.wikipedia.org/wiki/Box-Muller_transform
/// @param mean the mean of the distribution
/// @param stddev the standard deviation of the distribution
/// @return a sample of this distribution
/// @note Stole from (GPL) https://code.google.com/p/nll/source/browse/trunk/nll/math-distribution-gaussian.h
inline double randn( const double mean, const double stddev ){
   double u1 = ( static_cast<double> ( rand() ) + 1 ) / ( (double)RAND_MAX + 1 );
   double u2 = ( static_cast<double> ( rand() ) + 1 ) / ( (double)RAND_MAX + 1 );
   assert( -2 * log( u1 ) >= 0 );
   double t1 = sqrt( -2 * log( u1 ) ) * cos( 2 * M_PI * u2 );
   return mean + stddev * t1;
}

#ifdef WIN32
inline bool isnan(double a){ return (a!=a); }
#endif

void filter_depthscan::initParameters(RichParameterSet* pars){
    double noisedefault = 0.0025;
    if(model())
        noisedefault = model()->bbox().diagonal().norm() * .0025;

    pars->addParam( new RichInt("density", 5, "A ray every K-pixels", "How many rays to shoot? (clamped [1,inf])") );
    pars->addParam( new RichFloat("maxangle", 80, "Grazing TH < ", "Discard when above certain grazing angle (degrees)") );
    // pars->addParam( new RichBool("getnormal", true, "Sample Normals", "Store normals by reading them from the mesh") );
    pars->addParam( new RichFloat("znoise", noisedefault, "Z Sample Noise", "Perturb samples along Z direction with the given variance. Value initialized at 1% of bbox.") );
    // pars->addParam( new RichFloat("saveviewdir", 80, "Sample Viewpoint", "Store view directions") );
    // pars->addParam( new RichBool("triangulate", 80, "Triangulate samples", "Generates a mesh instead of a point cloud") );
    // pars->addParam( new RichFloat("", 80, "Triangulate samples", "Generates a mesh instead of a point cloud") );
}

void filter_depthscan::applyFilter(RichParameterSet* pars){
    typedef qglviewer::Vec QGLVec;
    /// Window size
    int w = drawArea()->width();
    int h = drawArea()->height();
   
    /// Create query octree
    SurfaceMeshModel* selection = SurfaceMesh::safe_cast( model() );
    Octree octree( selection );

    if(selection->n_faces()==0)
        throw StarlabException("Cannot scan a point cloud, need n_faces()>0");
    
    /// Need face normals for angle rejection
    if(!selection->has_face_normals())
        SurfaceMesh::NormalsHelper(selection).compute_face_normals();
    Vector3FaceProperty fnormals = selection->face_normals();
    
    /// Create a model to store scans & add it
    SurfaceMeshModel* scan = new SurfaceMeshModel("","Scan");
    document()->addModel(scan);   
    drawArea()->setRenderer(scan,"Flat Shading");
    scan->color = QColor(125,0,0);

    /// assert( step>1 )
    int step = qMax( pars->getInt("density"),1 );
    
    /// angle: [0,90]
    double angle = qBound(0.0, (double) pars->getFloat("maxangle"), 90.0);
    double mincosangle = cos( angle * M_PI / 180 );
    
    /// scan noise
    double znoise = pars->getFloat("znoise");
    
    /// Fetch normals?
    // bool getNormals = pars->getBool("getnormal");
    

    /// Size of matrices
    int nrows = std::floor( double(h) / step );
    int ncols = std::floor( double(w) / step );
    
    /// Layers containing point coordinates
    Matrix<XYZ_View, Dynamic, Dynamic> X(ncols,nrows);

    /// Perform scan
    for(int winX=0,i=0; i<ncols; winX+=step, i++){
        for(int winY=0,j=0; j<nrows; winY+=step, j++){
            QGLVec _orig, _dir;
            drawArea()->camera()->convertClickToLine( QPoint(winX, winY), _orig, _dir );
            Vector3 orig(_orig[0],_orig[1],_orig[2]);
            Vector3 dir(_dir[0],_dir[1],_dir[2]);
            dir.normalize(); ///< just to be sure
            int isectHit = -1;
            Eigen::Vector3d ipoint = octree.closestIntersectionPoint( Ray(orig, dir), &isectHit );
            
            X(i,j).xyz[0] = std::numeric_limits<Scalar>::quiet_NaN();
            if(isectHit>=0){
                Vector3 fnormal = fnormals[ Face(isectHit) ];
                double cosangle = std::abs(dot( fnormal, dir ));
                if(cosangle>mincosangle){
                    ipoint += dir*randn(0.0,znoise);
                    
                    /// Save point
                    X(i,j).xyz = ipoint;
                    X(i,j).view = dir;
                }
            }
        }
    }

    DepthTriangulator(scan).execute(X, mincosangle);
    scan->update_vertex_normals();
    
    /// Inform user
    showMessage("Scanned #P=%d points", scan->n_vertices());
}




/// Depth map based scan.. even by setting the depth map to 32 bits precision, 
/// we get shitty results..
#if 0
void filter_depthscan::applyFilter(RichParameterSet* /*pars*/){
    glEnable(GL_DEPTH_TEST);

    /// Window size
    int w = drawArea()->width();
    int h = drawArea()->height();

    /// Create a model to store scans & add it
    SurfaceMeshModel* model = new SurfaceMeshModel();
    document()->addModel(model);   
    
#if 0
    /// Query buffer precision
    QGLFormat format = drawArea()->format();
    format.setDepthBufferSize(32);
    drawArea()->setFormat(format);
    GLint bits;
    glGetIntegerv(GL_DEPTH_BITS, &bits);
    qDebug() << "depth buffer precision" << bits;
#endif 
    
#if 0
    drawArea()->updateGL();
#else
    /// Render essentials
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glPushMatrix();
        glMultMatrixd( document()->transform.data() );
        foreach(StarlabModel* model, document()->models())
            if(model->isVisible && model->renderer()!=NULL ){ 
                drawArea()->qglColor(model->color);
                model->renderer()->render();
            }
    glPopMatrix();
#endif
    
/// QGLViewer version will take ****ages**** to give you something
#if 0
    for(int winX=0; winX<w; winX++){
        for(int winY=0; winY<h; winY++){
            qDebug() << winX << winY;
            bool found=false;
            qglviewer::Vec p = drawArea()->camera()->pointUnderPixel(QPoint(winX, winY),found);
            if(found) model->add_vertex( Vector3(p[0], p[1], p[2]) );
        }
    }

/// Pure OpenGL version
#else
    /// Fetch the depth buffer (BOTTLENECK!!!!)
    /// http://stackoverflow.com/questions/516778/get-depth-buffer-from-qglpixelbuffer
    std::vector<GLfloat> winZv(w*h);
    GLfloat* winZ = winZv.data();
    glReadPixels(0, 0, w, h, GL_DEPTH_COMPONENT, GL_FLOAT, winZ);
    
    /// Fetch view matrices 
    GLint viewport[4];
    GLdouble modelview[16];
    GLdouble projection[16];
    glGetDoublev( GL_MODELVIEW_MATRIX, modelview );
    glGetDoublev( GL_PROJECTION_MATRIX, projection );
    glGetIntegerv( GL_VIEWPORT, viewport );
    
    /// Position of point in 3D space
    GLdouble posX, posY, posZ;
       
    /// Unproject depth buffer
    for(int winX=0; winX<w; winX++){
        for(int winY=0; winY<h; winY++){
            float depth = winZ[w*winY + winX];
            if(depth<1){
                gluUnProject(winX, winY, depth, modelview, projection, viewport, &posX, &posY, &posZ);
                model->add_vertex( Vector3(posX, posY, posZ) );
            }
        }
    }
   
    /// Don't affect visualization
    drawArea()->updateGL();    
#endif
}
#endif
