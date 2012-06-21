#include "mcfskel.h"

/// QT-stuff
#include <QList>
#include <QDir>
#include <QFileDialog>

/// Dictates the motion 
#include "ContractionHelper.h"
#include "PoleAttractorHelper.h"
/// Dictates topology changes
#include "TopologyJanitor.h"
/// I/O
#include "../starlab/plugins/surfacemesh_io_off/surfacemesh_io_helpers.h"

void mcfskel::initParameters(SurfaceMeshModel *mesh, RichParameterSet *parameters, StarlabDrawArea */*drawArea*/){
    /// Add a transparent copy of the model
#if 0
        SurfaceMeshModel* copy = new SurfaceMeshModel(mesh->path,"original");
        copy->read( mesh->path.toStdString() );
        document->addModel(copy);
        drawArea->initRendering();
        copy->renderer()->setRenderMode("Transparent");
#endif

    Scalar scale = 0.002*mesh->getBoundingBox().size().length();
    parameters->addParam(new RichFloat("omega_L_0",1));
    parameters->addParam(new RichFloat("omega_H_0",20));
    parameters->addParam(new RichFloat("omega_P_0",40));
    parameters->addParam(new RichFloat("edgelength_TH",scale));
    parameters->addParam(new RichFloat("zero_TH",1e-10));
}

void mcfskel::applyFilter(SurfaceMeshModel* mesh, RichParameterSet *pars, StarlabDrawArea *drawArea){
    if(firststep) mesh->renderer()->setRenderMode("Smooth");
    qDebug() << "==== Medial Contraction ===";
    qDebug() << "Step# : " << stepcount;

    ContractionHelper h(mesh);

    Scalar omega_L_0 = pars->getFloat("omega_L_0");
    Scalar omega_H_0 = pars->getFloat("omega_H_0");
    Scalar omega_P_0 = pars->getFloat("omega_P_0");
    Scalar edgelength_TH = pars->getFloat("edgelength_TH");
    Scalar zero_TH = pars->getFloat("zero_TH");

    /// Compute initialization
    ScalarVertexProperty varea_0    = mesh->vertex_property<Scalar>("v:area_0",1);
    Vector3VertexProperty points_0  = mesh->vertex_property<Vector3>("v:point_0");
    Vector3VertexProperty points    = mesh->vertex_property<Vector3>("v:point");
    ScalarVertexProperty omega_H    = mesh->vertex_property<Scalar>("v:omega_H",omega_H_0);
    ScalarVertexProperty omega_L    = mesh->vertex_property<Scalar>("v:omega_L",omega_L_0);
    BoolVertexProperty   vissplit   = mesh->vertex_property<bool>("v:issplit",false);
    BoolVertexProperty   visfixed   = mesh->vertex_property<bool>("v:isfixed",false);
    Vector3VertexProperty poles     = mesh->vertex_property<Vector3>("v:poles");
    ScalarVertexProperty omega_P	= mesh->vertex_property<Scalar>("v:omega_P",0);

    /// Init/retrieve properties
    static MeanHelper meanArea;
    if(firststep){
        /// Retrieves poles from the associated mesh
        typedef Surface_mesh::Vertex_property< QList<Vector3> > VSetVertexProperty;
        VSetVertexProperty pset = mesh->vertex_property< QList<Vector3> >("v:pset");
        SurfaceMeshModel polemesh;
        QString basename = QFileInfo(mesh->path).baseName();
        QString currpath = QFileInfo(mesh->path).dir().dirName();

        QString polesfullpath = QFileDialog::getOpenFileName(0, tr("Open pole"), "", tr("Pole Files (*.off)"));

        if(!QFileInfo(polesfullpath).exists()) throw StarlabException("Cannot find poles file at: %s",qPrintable(polesfullpath) );
        bool success = attempt_read_as_medial_mesh(polesfullpath,&polemesh);
        if(!success) throw StarlabException("Failed to open pole file");
        Vector3VertexProperty poles_in = polemesh.get_vertex_property<Vector3>("v:point");
        Q_ASSERT(poles_in);

        qDebug() << "Cache v:area_0";
        varea_0 = h.computeVertexVoronoiArea("v:area_0");
        foreach(Vertex v, mesh->vertices()){
            meanArea.push(varea_0[v]);
            points_0[v] = points[v];

            /// Save pole in currene mesh
            poles[v] = poles_in[v];
            /// The initial pole set is trivial
            pset[v].push_back(poles[v]);
        }
    }

#if 0
    /// Draw the initial cloud
    foreach(Vertex v, mesh->vertices()){
        // if(vissplit[v]) continue;
        drawArea->drawPoint(points_0[v],1);
    }
#endif

    /// ----------------------------------------------------------------------------- ///
    ///
    ///                               Solver section
    ///
    /// ----------------------------------------------------------------------------- ///
    if(true){
        if(firststep){
            foreach(Vertex v, mesh->vertices()){
                omega_L[v] = omega_L_0;
                omega_H[v] = omega_H_0;
                omega_P[v] = omega_P_0;
            }
        }

        /// Update Laplacian
        h.computeMeanValueHalfEdgeWeights(zero_TH);
        h.createLaplacianMatrix();

        /// Set constraints and solve
        try{
            h.solve(omega_H, omega_L, omega_P, poles);
        } catch(StarlabException e){}

        /// Update constraints
        {
            ScalarVertexProperty varea = h.computeVertexVoronoiArea(VAREA);
            foreach(Vertex v, mesh->vertices()){
                /// Leave fixed points really alone
                if(visfixed[v]){
                    omega_L[v] = 0;
                    omega_H[v] = 1.0/zero_TH;
                    omega_P[v] = 0;
                    continue;
                }

                omega_L[v] = omega_L_0; // *pow(varea[v]/meanArea, .15);
                omega_H[v] = omega_H_0; // *pow(varea[v]/meanArea,-alpha);
#ifdef USE_POLE_ANGLE
                omega_P[v] = omega_P_0 * angle_weight( pangle[v] );
#else
                omega_P[v] = omega_P_0;
#endif
                /// Ficticious vertices are simply relaxed
                if(vissplit[v]){
                    omega_L[v] = omega_L_0;
                    omega_H[v] = omega_H_0;
                    omega_P[v] = 0;
                }
            }
        }
    }

    /// ----------------------------------------------------------------------------- ///
    ///
    ///                         Topology management section
    ///
    /// ----------------------------------------------------------------------------- ///
    // qDebug() << TopologyJanitor(mesh).cleanup(1e-10,edgelength_TH,110);
    // qDebug() << TopologyJanitor_MergePoleSet(mesh).cleanup(1e-10,edgelength_TH,110);
    qDebug() << TopologyJanitor_ClosestPole(mesh).cleanup(zero_TH,edgelength_TH,110);


    /// ----------------------------------------------------------------------------- ///
    ///
    ///                                 VERTEX FIXER
    ///
    /// ----------------------------------------------------------------------------- ///
    Scalar elength_fixed = edgelength_TH/10.0;
    foreach(Vertex v, mesh->vertices()){
        if( visfixed[v] ){
            drawArea->drawPoint(points[v],3,Qt::red);
            continue;
        }
        bool willbefixed = false;
        Counter badcounter=0;
        foreach(Halfedge h, mesh->onering_hedges(v)){
            Scalar elength = mesh->edge_length(mesh->edge(h));
            if(elength<elength_fixed && !mesh->is_collapse_ok(h))
                badcounter++;
        }
        willbefixed = (badcounter>=2);
        visfixed[v] = willbefixed;
        if(willbefixed) drawArea->drawPoint(points[v],3,Qt::red);
    }
    /// ----------------------------------------------------------------------------- ///
    ///
    ///                                 STATISTICS
    ///
    /// ----------------------------------------------------------------------------- ///
    {
#ifdef USE_POLE_ANGLE
        /// Map pole attraction constraints to colors
        if(false){
            ScalarVertexProperty vquality = mesh->vertex_property<Scalar>(VQUALITY);
            foreach(Vertex v, mesh->vertices()){
                if(vissplit[v])
                    vquality[v] = 0;
                else
                    vquality[v] = omega_P[v];
            }
            ColorizeHelper(mesh).vscalar_to_vcolor(VQUALITY);
        }
#endif

        /// Show corresponding attracting poles
        if(false){
            /// Keep the set of poles associated with the point
            typedef QList<Vector3> PoleList;
            typedef Surface_mesh::Vertex_property<PoleList> VSetVertexProperty;
            VSetVertexProperty pset = mesh->vertex_property<PoleList>("v:pset");
            foreach(Vertex v, mesh->vertices())
                foreach( Vector3 pole, pset[v] )
                    drawArea->drawSegment(points[v], pole,1,Qt::gray);
        }
        /// Show ACTIVE attracting poles
        if(false){
            foreach(Vertex v, mesh->vertices())
                if(!vissplit[v]){
                    Vector3 pole  = poles[v];
                    Vector3 point = points[v];
                    drawArea->drawSegment(pole, point,3,Qt::green);
                }
        }
        /// Mark converged points
        if(false){
            ScalarVertexProperty varea = h.computeVertexVoronoiArea(VAREA);
            foreach(Vertex v, mesh->vertices()){
                if(varea[v]<1e-6)
                    drawArea->drawPoint(points[v],3,Qt::magenta);
            }
        }

        /// Show constraints stats
        if(false){
            qDebug() << "===== STATISTICS =====";
            qDebug() << ScalarStatisticsHelper(mesh).statistics("v:omega_H");
            qDebug() << ScalarStatisticsHelper(mesh).statistics("v:omega_L");
            qDebug() << ScalarStatisticsHelper(mesh).statistics("h:weight");
            qDebug() << ScalarStatisticsHelper(mesh).statistics("v:omega_P");
        }

        /// Colorize splits
        if(false){
            ScalarVertexProperty vquality = mesh->vertex_property<Scalar>(VQUALITY,0.0);
            foreach(Vertex v, mesh->vertices())
                vquality[v] = ((vissplit[v]) ? 1.0 : 0.0);
            ColorizeHelper(mesh).vscalar_to_vcolor(VQUALITY);
            qDebug() << ScalarStatisticsHelper(mesh).statistics(VQUALITY);
        }

        /// Debug edge lengths
        if(false){
            SurfaceMeshHelper(mesh).computeEdgeLengths(ELENGTH);
            qDebug() << ScalarStatisticsHelper(mesh).statistics(ELENGTH);
        }

        /// Map contraction constraints to colors
        if(false){
            ScalarVertexProperty vquality = mesh->vertex_property<Scalar>(VQUALITY);
            foreach(Vertex v, mesh->vertices())
                vquality[v] = (omega_L[v]);
            ColorizeHelper(mesh).vscalar_to_vcolor(VQUALITY);
        }

        /// Display # vertices
        if(false)
            qDebug() << "Current #vertices" << mesh->n_vertices();
    }

    /// Manage garbage
    /// @note IMPORTANT: code above might not be happy!!!!!
    if(false)
        mesh->garbage_collection();

    /// Change name and path of the file
    if(firststep){
        QFileInfo fi(mesh->path);
        QString basename = fi.baseName();
        QString currpath = fi.dir().path();
        mesh->path = currpath+"/"+basename+"_ckel.off";
        qDebug() << "Path changed!!";
    }
}

Q_EXPORT_PLUGIN(mcfskel)
