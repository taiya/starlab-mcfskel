#pragma once
#include "StarlabDrawArea.h"
#include "SurfaceMeshPlugins.h"
#include "SurfaceMeshHelper.h"

#include "arap_widget.h"
#include "ARAPDeformer.h"
#include "ARAPDeformerHandle.h"

enum SELECT_MODE{ADD, REMOVE, NONE};
enum ARAP_MODE{CONTROL_FACES, ANCHOR_FACES, DEFORM};
enum ANCHOR_MODE{MANUAL_POINTS, GEODESIC_DISTANCE};

class surfacemesh_mode_arapdeform : public SurfaceMeshModePlugin{
    Q_OBJECT
    Q_INTERFACES(ModePlugin)

public:
    surfacemesh_mode_arapdeform();

    QIcon icon(){ return QIcon(":/surfacemesh_mode_arapdeform.png"); }

    /// Functions part of the EditPlugin system
    void create();
    void destroy(){}

    bool endSelection(const QPoint& p);

    void decorate();
    void drawHandle();
    void drawWithNames();

	void update();

    /// User interactions
    bool wheelEvent(QWheelEvent *);
    bool mouseMoveEvent(QMouseEvent*);
    bool keyPressEvent(QKeyEvent *);

private:
    arap_widget * widget;
    QCursor paint_cursor, erase_cursor, deform_cursor;
    QPoint cursorPos;
    int brushSize;

    SELECT_MODE selectMode;
    ARAP_MODE currentMode;
    ANCHOR_MODE anchorMode;

    Vector3VertexProperty points, src_points;
    Vector3FaceProperty fnormals;

    QSet<Face> controlFaces;
    QSet<Face> anchorFaces;

    QSet<Vertex> controlPoints();
    QSet<Vertex> anchorPoints();

    ARAPDeformer * deformer;
    ARAPDeformerHandle * deform_handle;
    void initDeform();
    int numIterations;

    bool isDrawBrushSize;

    double anchorDistance;

public slots:
    void Deform();

    void setControlMode();
    void setAnchorMode();
    void setDeformMode();
    void setNumIterations(int);

    void setAcitveAnchorMode(int);
    void setDistanceAnchors(int);

    void compute_geodesic_anchors();
};


