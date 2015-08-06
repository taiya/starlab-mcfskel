#pragma once
#include "starlib_global.h"

#include <QObject>
#include <QList>
#include <QColor>

#include "qglviewer/qglviewer.h"
#include "RenderPlugin.h"
#include "RenderObject.h"
#include "StarlabMainWindow.h"

class QGLWidget;

namespace Starlab{

#ifndef GL_MULTISAMPLE
#define GL_MULTISAMPLE  0x809D
#endif

/// @ingroup starcore
/// @brief The QGLViewer based visualization window
class STARLIB_EXPORT DrawArea : public QGLViewer{
    Q_OBJECT
    
/// @{ static instancing
private:
    static DrawArea* _staticInstance;
public:
    /// @return Returns a pointer to the application's DrawArea instance.
    ///         if no instance has been allocated, NULL is returned
    static DrawArea* instance(){ return _staticInstance; }
/// @}    
    
/// @{ core
public:
    /// Constructor
    /// @par parent 
    DrawArea(MainWindow* parent);
    /// Polymorphic destructor
    ~DrawArea();
private:
    MainWindow* const _mainWindow;
public:
    MainWindow* mainWindow(){ return _mainWindow; }
    Document* document(){ return _mainWindow->document(); }
    Settings* settings(){ return _mainWindow->settings(); }
    PluginManager* pluginManager(){ return _mainWindow->pluginManager(); }
/// @}
 
/// @{ Rendering specification
public:
    void setRenderer(Model* model, QString pluginName);
    
public slots:
    void resetViewport();     ///< Restores the default view camera
    void viewFrom(QAction *); ///< Preset camera viewpoints
    void setPerspectiveProjection();
    void setOrthoProjection();
    void setIsoProjection();
private:
    void init();              ///< Initializes the scene (qGLViewer)
public:
    void draw();              ///< Draws the whole scene
    void draw_models();       ///< Draws ONLY the models in the scene (no decorators, no plugins, etc...) 
/// @}

/// @{ Selection / Picking
private:
    void drawWithNames();     ///< Draws to the symbols buffer
private:
    void endSelection(const QPoint&);
    void postSelection(const QPoint&);
/// @}
    
public:
    /// @see qglviewer::convertClickToLine(...). This is "EIGEN" compatible
    Ray3 convert_click_to_ray(const QPoint& pickpoint);

/// @{ ModePlugin management
private:
    bool eventFilter(QObject *object, QEvent *event);
/// @}

/// @{ Default mouse & keyboard management
    void mouseDoubleClickEvent( QMouseEvent * e );
/// @}
        
/// @{ Scene-wide rendering elements (i.e. used for debug)
public slots:
    /// @obsolete see clear()
    void deleteAllRenderObjects(){ qWarning("deleteAllRenderObject() obsolete!"); clear(); }
    /// @brief Removes all debug render objects from the scene
    void clear();
public:
    void drawAllRenderObjects();
    void deleteRenderObject(RenderObject* );
    void addRenderObject(RenderObject::Base*);
    RenderObject::Triangle& drawTriangle(Vector3 p1, Vector3 p2, Vector3 p3, QColor color=Qt::red);
    RenderObject::Point&    drawPoint(Vector3 p1, float size=1, QColor color=Qt::red);
    RenderObject::Segment&  drawSegment(Vector3 p1, Vector3 p2, float size=1, QColor color=Qt::red);
    RenderObject::Ray&      drawRay(Vector3 orig, Vector3 dir, float size=1, QColor color=Qt::red, float scale=1);
    RenderObject::Text&     drawText(int x, int y, const QString& text, float size=12, QColor color=Qt::red);
/// @}
    
public slots:
    /// Updates the draw area, this is not the OpenGL update, this updates all
    /// the metadata needed by all models for correct rendering!! 
    void update();

protected:
    /// Handles to additional render objects, created by draw??() methods, deleted by ~StarlabDrawArea()
    QList<RenderObject::Base*> renderObjectList;
};

} // namespace

/// Concatenated Namespace
typedef Starlab::DrawArea StarlabDrawArea;
