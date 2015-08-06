#include <QGLWidget>
#include <QMouseEvent>
#include <QColorDialog>

#include "StarlabDrawArea.h"
#include "Document.h"
#include "StarlabException.h"
#include "RenderObject.h"
#include "ModePlugin.h"
#include "DecoratePlugin.h"

using namespace Starlab;
using namespace qglviewer;

/// By default the static instance is NULL
DrawArea* DrawArea::_staticInstance = NULL;

void DrawArea::update(){
    // qDebug() << "StarlabDrawArea::update()";
    
    /// @internal Initialization can act on busy document
    /// Update the metadata needed by the renderer
    /// e.g. stick data in vertex buffer.. etc..
    foreach(Model* model, document()->models()){
        /// Create instance if renderer missing
        if(model->renderer()==NULL){
            QString name = pluginManager()->getPreferredRenderer(model);
            RenderPlugin* plugin = pluginManager()->getRenderPlugin(name);
            model->setRenderer( plugin );
        } else {
            /// just re-initialize it
            model->renderer()->init();
        }
    }
    
    /// Don't update on a busy document
    if(document()->isBusy()) 
        return;

    /// This will force a "paint" of the GL window
    updateGL();
}

DrawArea::DrawArea(MainWindow* parent) 
    : QGLViewer(parent), _mainWindow(parent){

    /// When document changes, refresh the rendering
    connect(document(), SIGNAL(hasChanged()), this, SLOT(update()));
    /// Determines whether events are forwarded to Mode plugins
    installEventFilter(this);
        
    /// @todo restore trackball
    // settings()->setDefault("StarlabDrawArea/showtrackball",false);

    /// Default camera
    camera()->setType(Camera::ORTHOGRAPHIC);
    
    /// Value of 100.0 forbids spinning
    camera()->frame()->setSpinningSensitivity(100);

    /// Disable saving state to file
    setStateFileName( QString::null );
    
    /// @todo setup View->ToggleFPS
    /// @todo why is update broken when nothing is moving?
    this->setFPSIsDisplayed(true);

    // setGridIsDrawn(false);
    
    /// @todo Add screenshot to View->Take Screenshot
    // Keyboard + Mouse behavior
    // setShortcut(SAVE_SCREENSHOT, Qt::CTRL + Qt::SHIFT + Qt::Key_S);

    /// Disables all default qglviewer predefined keyboard shortcuts
    {
        setShortcut(DRAW_AXIS,0);
        setShortcut(DRAW_GRID,0);
        setShortcut(DISPLAY_FPS,0);
        setShortcut(STEREO,0);
        setShortcut(HELP,0);
        setShortcut(EXIT_VIEWER,0);
        setShortcut(CAMERA_MODE,0);
        setShortcut(ANIMATION,0);
        setShortcut(SAVE_SCREENSHOT,0);
        setShortcut(FULL_SCREEN,0);
        setShortcut(EDIT_CAMERA,0);
        setShortcut(INCREASE_FLYSPEED,0);
        setShortcut(DECREASE_FLYSPEED,0);
        setShortcut(MOVE_CAMERA_LEFT,0);
        setShortcut(MOVE_CAMERA_RIGHT,0);
        setShortcut(MOVE_CAMERA_UP,0);
        setShortcut(MOVE_CAMERA_DOWN,0);
    }
    
    /// Saves a static instance
    Q_ASSERT(DrawArea::_staticInstance==NULL);
    DrawArea::_staticInstance = this;
}

DrawArea::~DrawArea(){
    // qDebug() << "~DrawArea()";
    qDeleteAll(renderObjectList.begin(), renderObjectList.end());
    renderObjectList.clear();
}

void DrawArea::resetViewport(){

    Eigen::AlignedBox3d sceneBBox = document()->bbox();
    Eigen::Vector3d minbound = sceneBBox.min();
    Eigen::Vector3d maxbound = sceneBBox.max();

    Vec min_bound(minbound.x(),minbound.y(),minbound.z());
    Vec max_bound(maxbound.x(),maxbound.y(),maxbound.z());

    camera()->fitBoundingBox( min_bound, max_bound );
    camera()->setSceneRadius((max_bound - min_bound).norm() * 0.4);
    camera()->setSceneCenter((min_bound + max_bound) * 0.5);
    camera()->showEntireScene();
}

void DrawArea::setPerspectiveProjection(){
    camera()->setType(Camera::PERSPECTIVE); updateGL();
}

void DrawArea::setOrthoProjection(){
    camera()->setType(Camera::ORTHOGRAPHIC); updateGL();
}

void DrawArea::setIsoProjection(){
    setOrthoProjection();

    // Move camera such that entire scene is visisble
    double r = camera()->sceneRadius();
    Frame f(camera()->sceneCenter() + Vec(r,-r,r), Quaternion());
    f.rotate(Quaternion(Vec(0,0,1), M_PI / 4.0));
    f.rotate(Quaternion(Vec(1,0,0), M_PI / 3.3));
    camera()->interpolateTo(f,0.25);
}

void DrawArea::viewFrom(QAction * a){

    if(!document()->selectedModel()) return;

    Frame f;

    QStringList list;
    list << "Top" << "Bottom" << "Left" << "Right" << "Front" << "Back";

    document()->selectedModel()->updateBoundingBox();
    Eigen::AlignedBox3d bbox = document()->selectedModel()->bbox();

    double e = bbox.diagonal().norm()*2;
    Vec c(bbox.center().x(),bbox.center().y(),bbox.center().z());

    switch(list.indexOf(a->text()))
    {
    case 0:
        f = Frame(c+Vec(0,0,e), Quaternion());
        break;
    case 1:
        f = Frame(c+Vec(0,0,-e), Quaternion());
        f.rotate(Quaternion(Vec(1,0,0), M_PI));
        break;
    case 2:
        f = Frame(c+Vec(0,-e,0), Quaternion(Vec(0,0,1),Vec(0,-1,0)));
        break;
    case 3:
        f = Frame(c+Vec(0,e,0), Quaternion(Vec(0,0,1),Vec(0,1,0)));
        f.rotate(Quaternion(Vec(0,0,1), M_PI));
        break;
    case 4:
        f = Frame(c+Vec(e,0,0), Quaternion(Vec(0,0,1),Vec(1,0,0)));
        f.rotate(Quaternion(Vec(0,0,1), M_PI / 2.0));
        break;
    case 5:
        f = Frame(c+Vec(-e,0,0), Quaternion(Vec(0,0,-1),Vec(1,0,0)));
        f.rotate(Quaternion(Vec(0,0,-1), M_PI / 2.0));
        break;
    }
    camera()->interpolateTo(f,0.25);

    camera()->setSceneCenter(c);
}

void DrawArea::init(){   
    /// Background color from settings file
    QString key = "DefaultBackgroundColor";

    QColor white_transp(255,255,255,255);
    settings()->setDefault( key, QVariant(white_transp) );
    setBackgroundColor( settings()->getQColor(key) );
    camera()->setUpVector(Vec(0,1,0));

    resetViewport();
}

void DrawArea::draw_models(){
    /// Render each Model
    /// @todo use plugin rendering if one is specified
    glPushMatrix();
        // glMultMatrixd( document()->transform.data() );
        foreach(Model* model, document()->models())
            if(model->isVisible && model->renderer()!=NULL ){ 
                qglColor(model->color);
                model->renderer()->render();
            }
    glPopMatrix();
}

void DrawArea::draw(){
    glEnable(GL_MULTISAMPLE); ///< Enables anti-aliasing

    /// Draw the models
    draw_models();
    
    /// @todo Render decoration plugins
    glPushMatrix();
        // glMultMatrixd( document()->transform.data() );

        /// @todo use the plugin decorators
        if(mainWindow()->hasModePlugin() && !mainWindow()->isModePluginSuspended())
            mainWindow()->getModePlugin()->decorate();            
        
        /// @todo use the standard decorators
        else{
            foreach(Model* model, document()->models()){
                if(model->isVisible) 
                    foreach(DecoratePlugin* decorator, model->decoratePlugins())
                        decorator->decorate();
            }
        }
    glPopMatrix();        

        
    /// Render mode decoration
    if( mainWindow()->hasModePlugin() )
        mainWindow()->getModePlugin()->decorate();
    
    /// Render renderable objects
    drawAllRenderObjects();
    
    /// Restore foreground color (the one of showMessage)
    setForegroundColor(Qt::black);
}

void DrawArea::drawWithNames(){
    if(mainWindow()->hasModePlugin())
        mainWindow()->getModePlugin()->drawWithNames();
}

void DrawArea::endSelection(const QPoint & p){
    /// We are forced to to this as QGLViewer has these methods as "protected"
    /// so they will never be visible in ModePlugin
    if(mainWindow()->hasModePlugin()){
        bool filtered = mainWindow()->getModePlugin()->endSelection(p);
        if(filtered) return;
    } 
    QGLViewer::endSelection(p);
}

void DrawArea::postSelection(const QPoint & p){
    /// We are forced to to this as QGLViewer has these methods as "protected"
    /// so they will never be visible in ModePlugin
    if(mainWindow()->hasModePlugin()){
        bool filtered = mainWindow()->getModePlugin()->postSelection(p);
        if(filtered) return;
    }
    QGLViewer::postSelection(p);
}

void DrawArea::setRenderer(Model *model, QString name){
    // qDebug("StarlabDrawArea::setRenderer(%s,%s)",qPrintable(model->name), qPrintable(pluginName));
    document()->pushBusy();
        RenderPlugin* plugin = pluginManager()->getRenderPlugin(name);
        model->setRenderer( plugin );
    document()->popBusy();
}

void DrawArea::clear(){
    qDeleteAll(renderObjectList.begin(), renderObjectList.end());
    renderObjectList.clear();
    updateGL();
}

void DrawArea::drawAllRenderObjects(){
    foreach(RenderObject::Base* obj, renderObjectList)
        obj->draw(*this);
}

void DrawArea::addRenderObject(RenderObject::Base * obj)
{
    renderObjectList.append(obj);
}


/// @todo add polygon drawing?
#if 0
for(uint i=0; i<poly.size()-2; i++){
    Point_2 p0 = poly[0]; // pivot 
    Point_2 p1 = poly[(i+1)%poly.size()];
    Point_2 p2 = poly[(i+2)%poly.size()];
    StarlabDrawArea::instance()->drawTriangle( toQt(p0), toQt(p1), toQt(p2) );
}
#endif

RenderObject::Triangle& DrawArea::drawTriangle(Vector3 p1, Vector3 p2, Vector3 p3, QColor color){
    RenderObject::Triangle* triangle = new RenderObject::Triangle(p1,p2,p3,color);
    addRenderObject(triangle);
    return *triangle;
}

RenderObject::Point& DrawArea::drawPoint(Vector3 p1, float size, QColor color){
    RenderObject::Point* point = new RenderObject::Point(p1,size,color);
    addRenderObject(point);
    return *point;
}

RenderObject::Segment& DrawArea::drawSegment(Vector3 p1, Vector3 p2, float size, QColor color){
    RenderObject::Segment* segment = new RenderObject::Segment(p1,p2,size,color);
    addRenderObject(segment);
    return *segment;
}

RenderObject::Ray& DrawArea::drawRay(Vector3 orig, Vector3 dir, float size, QColor color, float scale){
    RenderObject::Ray* ray = new RenderObject::Ray(orig,dir,size,color,scale);
    addRenderObject(ray);
    return *ray;
}

RenderObject::Text& DrawArea::drawText(int x, int y, const QString& text, float size, QColor color){
    RenderObject::Text* t = new RenderObject::Text(x, y, text, size, color);
    addRenderObject(t);
    return *t;
}

/// @internal returning true will prevent the drawArea plugin from intercepting the events!!!
bool DrawArea::eventFilter(QObject*, QEvent* event){
    /// If a mode is not open, pass *everything* to the drawArea plugin
    if(!mainWindow()->hasModePlugin()) return false;
    /// Same if the mode plugin is suspended
    if(mainWindow()->isModePluginSuspended()) return false;

    /// If it is open, pass it to the handlers
    ModePlugin* mode = mainWindow()->getModePlugin();
    switch(event->type()){
        case QEvent::MouseButtonRelease: return mode->mouseReleaseEvent((QMouseEvent*)event); break;
        case QEvent::MouseButtonPress:   return mode->mousePressEvent((QMouseEvent*)event); break;
        case QEvent::MouseMove:          return mode->mouseMoveEvent((QMouseEvent*)event); break;
        case QEvent::KeyPress:           return mode->keyPressEvent((QKeyEvent*)event); break;
        case QEvent::KeyRelease:         return mode->keyReleaseEvent((QKeyEvent*)event); break;
        case QEvent::Wheel:              return mode->wheelEvent((QWheelEvent*)event); break;
        case QEvent::MouseButtonDblClick:return mode->mouseDoubleClickEvent((QMouseEvent *)event); break;
        default: return false;
    }
}


void DrawArea::mouseDoubleClickEvent(QMouseEvent *e)
{
    bool found = false;
    qglviewer::Vec p = camera()->pointUnderPixel(e->pos(), found);

    if(found){
        camera()->setRevolveAroundPoint(p);
        camera()->lookAt(p);
        // Log the position
        mainWindow()->setStatusBarMessage(QString("Arcball centered at x = %1 | y = %2 | z = %3").arg(p.x).arg(p.y).arg(p.z));
    } 
    
#if 0 
    // Regaular behavior when clicking background
    else {
        QGLViewer::mouseDoubleClickEvent(e);
        camera()->setRevolveAroundPoint(this->sceneCenter());
        camera()->showEntireScene();        
    }
#endif
}

void DrawArea::deleteRenderObject(RenderObject* /*object*/){
    /// @todo 
    throw StarlabException("TODO: StarlabDrawArea::deleteRenderObject");
}

Ray3 DrawArea::convert_click_to_ray(const QPoint& pickpoint){
    qglviewer::Vec _orig, _dir;
    this->camera()->convertClickToLine(pickpoint,_orig,_dir);
    Vector3 orig(_orig[0],_orig[1],_orig[2]);
    Vector3 dir(_dir[0],_dir[1],_dir[2]);
    dir.normalize(); ///< just to be sure?
    return Ray3(orig, dir);
}
