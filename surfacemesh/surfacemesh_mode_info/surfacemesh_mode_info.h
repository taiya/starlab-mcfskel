#pragma once
#include "SurfaceMeshPlugins.h"
#include "SurfaceMeshHelper.h"
#include "StarlabDrawArea.h"

enum DrawElementType{VERT_IDX, FACE_IDX, EDGE_IDX, HDGE_IDX};

using namespace SurfaceMesh;

class surfacemesh_mode_info : public SurfaceMeshModePlugin{
    Q_OBJECT
    Q_INTERFACES(ModePlugin)

    QIcon icon(){ return QIcon(":/images/cursor-question.png"); }
    QImage fontImage;

    /// Functions part of the EditPlugin system
    void create();
    void destroy(){}
    bool documentChanged();

    void decorate();
    void drawWithNames();
    bool postSelection(const QPoint& p);
    bool endSelection(const QPoint& p);

    void drawIndex(DrawElementType, QColor, double vt = -0.4);

    void beginDrawIndex();
    void drawIndexVertex(Vertex,bool shadow=false);
    void drawIndexFace(Face,bool shadow=false);
    void drawIndexEdge(int, Vector3, bool shadow=false);
    void endDrawIndex();

    void drawSelectedItem();
    void drawItemInfo();

    qglviewer::Vec cameraProjection(Vector3);

    Vector3VertexProperty points;
    Vector3FaceProperty faceCenters;
    Vector3FaceProperty faceNormals;
    ScalarFaceProperty faceAreas;
    ScalarEdgeProperty elengs;

    DrawElementType selectedType;
    int selectedIdx;
    int correctIndex(int i);

    QVector<bool> visualize;

public:
    virtual bool keyReleaseEvent(QKeyEvent* event);
    virtual bool keyPressEvent (QKeyEvent* event);
    void update();
};

// Utility
#define qRanged(min, v, max) ( qMax(min, qMin(v, max)) )
#define RADIANS(deg)    ((deg)/180.0 * M_PI)
#define DEGREES(rad)    ((rad)/M_PI * 180.0)
