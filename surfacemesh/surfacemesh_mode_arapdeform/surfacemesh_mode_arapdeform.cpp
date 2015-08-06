#include <QMouseEvent>
#include "SurfaceMeshPlugins.h"
#include "ModePluginDockWidget.h"
#include "surfacemesh_mode_arapdeform.h"
#include "StarlabDrawArea.h"
#include "arap_globals.h"

static uint qHash( const Face &key ){return qHash(key.idx()); }
static uint qHash( const Vertex &key ){return qHash(key.idx()); }

surfacemesh_mode_arapdeform::surfacemesh_mode_arapdeform()
{
    this->widget = NULL;
    this->deformer = NULL;
    this->deform_handle = NULL;

	if(!QApplication::topLevelWidgets().isEmpty())
	{
		paint_cursor = QCursor(QPixmap(":/images/paint.png"), 0, 32);
		erase_cursor = QCursor(QPixmap(":/images/erase.png"), 0, 32);
		deform_cursor = QCursor(QPixmap(":/images/deform_cursor.png"), 0, 0);
	}

    brushSize = 10;
    isDrawBrushSize = false;

    this->anchorMode = MANUAL_POINTS;
    this->numIterations = 1;
    this->anchorDistance = 0.5;
}

void surfacemesh_mode_arapdeform::create()
{
    if(!widget)
    {
        ModePluginDockWidget * dockwidget = new ModePluginDockWidget("As-rigid-as-possible deformation", mainWindow());
        widget = new arap_widget(this);
        dockwidget->setWidget(widget);
        mainWindow()->addDockWidget(Qt::RightDockWidgetArea,dockwidget);
    }

    update();
}

void surfacemesh_mode_arapdeform::update()
{
    setControlMode();

    points = mesh()->vertex_property<Vector3>(VPOINT);
    fnormals = mesh()->face_property<Vector3>(FNORMAL);
}

bool surfacemesh_mode_arapdeform::endSelection(const QPoint &)
{
    glFlush();
    GLint nbHits = glRenderMode(GL_RENDER);

    QSet<int> selection;

    if (nbHits > 0)
        for (int i=0; i<nbHits; ++i)
            selection.insert((drawArea()->selectBuffer())[4*i+3]);

    foreach(int idx, selection){
        Face f(idx);

        switch(currentMode)
        {
            case CONTROL_FACES:
                if(selectMode == ADD){
                    controlFaces.insert(f);
                    anchorFaces.remove(f);
                }
                if(selectMode == REMOVE){
                    controlFaces.remove(f);
                }
                break;

            case ANCHOR_FACES:
                if(selectMode == ADD) {anchorFaces.insert(f); controlFaces.remove(f);  }
                if(selectMode == REMOVE) anchorFaces.remove(f);
                break;
            case DEFORM:
                break;
        }
    }

    return true;
}

bool surfacemesh_mode_arapdeform::wheelEvent(QWheelEvent * e)
{
    cursorPos = e->pos();
    if(e->modifiers() == Qt::NoButton) return false;

    double s = e->delta() / 120.0;
    brushSize += s;

    drawArea()->setSelectRegionHeight(brushSize);
    drawArea()->setSelectRegionWidth(brushSize);

    isDrawBrushSize = true;
    drawArea()->updateGL();

    return false;
}

bool surfacemesh_mode_arapdeform::mouseMoveEvent(QMouseEvent * e)
{
    cursorPos = e->pos();
    if(e->modifiers() == Qt::NoButton || currentMode == DEFORM) return false;

    if(e->buttons() & Qt::LeftButton)   selectMode = ADD;
    if(e->buttons() & Qt::RightButton)  selectMode = REMOVE;

    switch(selectMode)
    {
        case ADD: drawArea()->setCursor(paint_cursor); break;
        case REMOVE: drawArea()->setCursor(erase_cursor); break;
        case NONE: drawArea()->setCursor(Qt::ArrowCursor); break;
    }

    switch(currentMode)
    {
        case CONTROL_FACES:
        case ANCHOR_FACES:
            drawArea()->select(e->pos());
            isDrawBrushSize = true;
        break;
        case DEFORM: break;            
    }

    drawArea()->updateGL();
    return true;
}

bool surfacemesh_mode_arapdeform::keyPressEvent(QKeyEvent *)
{
    return false;
}

void surfacemesh_mode_arapdeform::decorate()
{
    glEnable(GL_LIGHTING);
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(-10, 10);

    // Control faces
    foreach(Face f, controlFaces){
        glColor3d(0,1,0);

        glBegin(GL_POLYGON);
        glNormal3dv(fnormals[f].data());
        Surface_mesh::Vertex_around_face_circulator vit, vend;
        vit = vend = mesh()->vertices(f);
        do{ glVertex3dv(points[vit].data()); } while(++vit != vend);
        glEnd();
    }

    // Anchor faces
    foreach(Face f, anchorFaces){
        glColor3d(1,0,0);

        glBegin(GL_POLYGON);
        glNormal3dv(fnormals[f].data());
        Surface_mesh::Vertex_around_face_circulator vit, vend;
        vit = vend = mesh()->vertices(f);
        do{ glVertex3dv(points[vit].data()); } while(++vit != vend);
        glEnd();
    }

    glDisable(GL_POLYGON_OFFSET_FILL);

    // Visualize brush size
    if(isDrawBrushSize)
    {
        drawArea()->startScreenCoordinatesSystem();
        glDisable(GL_LIGHTING);
        glColor4d(1,1,1,0.5);
        glLineWidth(1.0f);
        DrawCircle(cursorPos.x(), cursorPos.y(), brushSize);
        drawArea()->stopScreenCoordinatesSystem();
    }

    QString info = "";

    switch(currentMode){
        case CONTROL_FACES: info += "Mode: Control points"; break;
        case ANCHOR_FACES: info += "Mode: Anchor points"; break;
        case DEFORM:
        {
            info += "Mode: Deform";
            drawHandle();
            break;
        }
    }

    switch(anchorMode){
        case MANUAL_POINTS: info += ", manual anchor"; break;
        case GEODESIC_DISTANCE: info += ", distance anchors = " + QString::number(anchorDistance); break;
    }

    glColor4d(1,1,1,1);
    drawArea()->renderText(50,50, info);

    glEnable(GL_LIGHTING);
}

void surfacemesh_mode_arapdeform::drawHandle()
{
    if(!deform_handle) return;

    int pixels = 32;
    double scaling = pixels * drawArea()->camera()->pixelGLRatio(Vec(0,0,0));

    glPushMatrix();
    glMultMatrixd(deform_handle->matrix());

    glDisable(GL_LIGHTING);
    glLineWidth(4.0f);

    glBegin(GL_LINES);

    glColor3d(1,0,0);
    glVertex3fv(Vec(0,0,0));
    glVertex3fv(Vec(1,0,0) * scaling);

    glColor3d(0,1,0);
    glVertex3fv(Vec(0,0,0));
    glVertex3fv(Vec(0,1,0) * scaling);

    glColor3d(0,0,1);
    glVertex3fv(Vec(0,0,0));
    glVertex3fv(Vec(0,0,1) * scaling);

    glEnd();
    glEnable(GL_LIGHTING);

    glPopMatrix();
}

void surfacemesh_mode_arapdeform::drawWithNames()
{
    double vt = 0;

    Vec viewDir = drawArea()->camera()->viewDirection().unit();
    Vector3 cameraNormal(viewDir[0],viewDir[1],viewDir[2]);

    foreach(Face f, mesh()->faces())
    {
        if(dot(fnormals[f], cameraNormal) > vt) continue;

        glPushName(f.idx());
        glBegin(GL_POLYGON);
        Surface_mesh::Vertex_around_face_circulator vit, vend;
        vit = vend = mesh()->vertices(f);
        do{ glVertex3dv(points[vit].data()); } while(++vit != vend);
        glEnd();
        glPopName();
    }
}

QSet<Vertex> surfacemesh_mode_arapdeform::controlPoints()
{
    QSet<Vertex> cpoints;

    foreach(Face f, controlFaces){
        Surface_mesh::Vertex_around_face_circulator vit, vend;
        vit = vend = mesh()->vertices(f);
        do{ cpoints.insert(vit); } while(++vit != vend);
    }

    return cpoints;
}

QSet<Vertex> surfacemesh_mode_arapdeform::anchorPoints()
{
    QSet<Vertex> apoints;

    foreach(Face f, anchorFaces){
        Surface_mesh::Vertex_around_face_circulator vit, vend;
        vit = vend = mesh()->vertices(f);
        do{ apoints.insert(vit); } while(++vit != vend);
    }

    return apoints;
}

void surfacemesh_mode_arapdeform::initDeform()
{
    // Check if everything is set and ready
    if(!controlFaces.size() || !anchorFaces.size()){
        setControlMode();
        drawArea()->displayMessage("Please add both control and anchor regions.");
        drawArea()->updateGL();
        return;
    }

    // Copy original mesh geometry
    src_points = mesh()->vertex_property<Vector3>("v:src_points");
    foreach(Vertex v, mesh()->vertices()) src_points[v] = points[v];

    if(deformer) delete deformer;

    // Create new As-rigid-as-possible deformer
    deformer = new ARAPDeformer(mesh());

    // Set anchor points
    foreach(Vertex v, anchorPoints())
        deformer->SetAnchor(v);

    // Set control points and find handle center
    QSet<Vertex> cpoints = controlPoints();
    Vector3 handleCenter(0,0,0);

    foreach(Vertex v, cpoints){
        deformer->UpdateControl(v, points[v]);
        handleCenter += points[v];
    }

    handleCenter /= cpoints.size();

    double radius = 0;
    foreach(Vertex v, cpoints){
        double d = (points[v] - handleCenter).norm();
        if(d > radius) radius = d;
    }

    // setup deformation handle
    deform_handle = new ARAPDeformerHandle( handleCenter, radius );
    drawArea()->setManipulatedFrame( deform_handle );
    this->connect(deform_handle, SIGNAL(manipulated()), SLOT(Deform()));
}

void surfacemesh_mode_arapdeform::Deform()
{
    // Update control positions
    foreach(Vertex v, controlPoints())
        points[v] = deform_handle->transform( src_points[v] );

    deformer->Deform( numIterations );

    mesh()->update_face_normals();
    mesh()->update_vertex_normals();
}

void surfacemesh_mode_arapdeform::setControlMode(){
    if(currentMode == DEFORM){
        controlFaces.clear();
        anchorFaces.clear();
    }

    currentMode = CONTROL_FACES;
    selectMode = ADD;
    drawArea()->updateGL();
}

void surfacemesh_mode_arapdeform::setAnchorMode(){
    if(currentMode == DEFORM){
        controlFaces.clear();
        anchorFaces.clear();
    }

    currentMode = ANCHOR_FACES;
    selectMode = ADD;
    drawArea()->updateGL();
}

void surfacemesh_mode_arapdeform::setDeformMode(){
    currentMode = DEFORM;
    selectMode = NONE;
    initDeform();

    drawArea()->setCursor(deform_cursor);
    isDrawBrushSize = false;

    drawArea()->updateGL();
}

void surfacemesh_mode_arapdeform::setNumIterations(int iters)
{
    this->numIterations = iters;
}

void surfacemesh_mode_arapdeform::setDistanceAnchors(int d)
{
    this->anchorDistance = double(d) / 100;
    this->compute_geodesic_anchors();
    drawArea()->updateGL();
}

void surfacemesh_mode_arapdeform::setAcitveAnchorMode(int mode)
{
    anchorMode = (ANCHOR_MODE)mode;

    switch(anchorMode)
    {
    case MANUAL_POINTS:
        anchorFaces.clear();
        break;

    case GEODESIC_DISTANCE:
        compute_geodesic_anchors();
        break;
    }

    drawArea()->updateGL();
}

void surfacemesh_mode_arapdeform::compute_geodesic_anchors()
{
    if(controlFaces.isEmpty()) return;

    // Tag source points
    BoolVertexProperty src = mesh()->vertex_property<bool>("v:geo_src_points", false);
    foreach(Vertex v, mesh()->vertices()) src[v] = false;
    foreach(Vertex v, controlPoints()) src[v] = true;

    // Compute distance
    QString g = "Geodesic distance: heat kernel";
    pluginManager()->getFilter(g)->applyFilter(NULL);
    ScalarVertexProperty geo_dist = mesh()->vertex_property<Scalar>("v:uniformDistance", 0);

    // Threshold and assign anchors faces
    anchorFaces.clear();

    foreach(Vertex v, mesh()->vertices()){
        if(geo_dist[v] > anchorDistance)
            geo_dist[v] = 1;
        else
            geo_dist[v] = 0;
    }

    QSet<Vertex> anchor_pnts;

    foreach(Edge e, mesh()->edges()){
        Vertex v1 = mesh()->vertex(e, 0);
        Vertex v2 = mesh()->vertex(e, 1);
        if(geo_dist[v1] + geo_dist[v2] < 2){
            if(geo_dist[v1]) anchor_pnts.insert(v1);
            if(geo_dist[v2]) anchor_pnts.insert(v2);
        }
    }

    foreach(Vertex v, anchor_pnts){
        Surface_mesh::Face_around_vertex_circulator adjF(mesh(), v), fend = adjF;
        do { anchorFaces.insert(adjF); } while(++adjF != fend);
    }
}

Q_EXPORT_PLUGIN(surfacemesh_mode_arapdeform)
