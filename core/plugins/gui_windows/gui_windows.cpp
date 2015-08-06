#include "gui_windows.h"
Q_EXPORT_PLUGIN(gui_windows)

// #include "saveSnapshotDialog.h"

// layerDialog->setVisible( !layerDialog->isVisible() );

/// On project save
#if 0
    layerDialog->setWindowTitle(fileName);
#endif

/// Save snapshot
//SaveSnapshotDialog dialog(this);
//dialog.setValues(GLA()->ss);
//if (dialog.exec()==QDialog::Accepted) {
//    GLA()->ss=dialog.getValues();
//    GLA()->saveSnapshot();
//    return true;
//}
//return false;

//void GLArea::saveSnapshot() {
///// @todo reenable snapshots
//#if 0
//    // snap all layers
//    currSnapLayer=0;
//    // number of subparts
//    totalCols=totalRows=ss.resolution;
//    tileRow=tileCol=0;
//    if(ss.snapAllLayers) {
//        while(currSnapLayer<this->md()->meshList.size()) {
//            tileRow=tileCol=0;
//            qDebug("Snapping layer %i",currSnapLayer);
//            this->md()->setCurrentMesh(currSnapLayer);
//            foreach(MeshModel *mp,this->md()->meshList) {
//                meshSetVisibility(mp,false);
//            }
//            meshSetVisibility(mm(),true);
//            takeSnapTile=true;
//            repaint();
//            currSnapLayer++;
//        }
//        //cleanup
//        foreach(MeshModel *mp,this->md()->meshList) {
//            meshSetVisibility(mp,true);
//        }
//        ss.counter++;
//    } else {
//        takeSnapTile=true;
//        update();
//    }
//#endif
//}

#if 0
void GLArea::pasteTile() {
    QString outfile;

    glPushAttrib(GL_ENABLE_BIT);
    QImage tileBuffer=grabFrameBuffer(true).mirrored(false,true);
    if(ss.tiledSave) {
        outfile=QString("%1/%2_%3-%4.png")
                .arg(ss.outdir)
                .arg(ss.basename)
                .arg(tileCol,2,10,QChar('0'))
                .arg(tileRow,2,10,QChar('0'));
        tileBuffer.mirrored(false,true).save(outfile,"PNG");
    } else {
        if (snapBuffer.isNull())
            snapBuffer = QImage(tileBuffer.width() * ss.resolution, tileBuffer.height() * ss.resolution, tileBuffer.format());

        uchar *snapPtr = snapBuffer.bits() + (tileBuffer.bytesPerLine() * tileCol) + ((totalCols * tileRow) * tileBuffer.numBytes());
        uchar *tilePtr = tileBuffer.bits();

        for (int y=0; y < tileBuffer.height(); y++) {
            memcpy((void*) snapPtr, (void*) tilePtr, tileBuffer.bytesPerLine());
            snapPtr+=tileBuffer.bytesPerLine() * totalCols;
            tilePtr+=tileBuffer.bytesPerLine();
        }
    }
    tileCol++;

    if (tileCol >= totalCols) {
        tileCol=0;
        tileRow++;

        if (tileRow >= totalRows) {
            if(ss.snapAllLayers) {
                outfile=QString("%1/%2%3_L%4.png")
                        .arg(ss.outdir).arg(ss.basename)
                        .arg(ss.counter,2,10,QChar('0'))
                        .arg(currSnapLayer,2,10,QChar('0'));
            } else {
                outfile=QString("%1/%2%3.png")
                        .arg(ss.outdir).arg(ss.basename)
                        .arg(ss.counter++,2,10,QChar('0'));
            }

            if(!ss.tiledSave) {
                bool ret = (snapBuffer.mirrored(false,true)).save(outfile,"PNG");
                if (ret) log->Logf(GLLogStream::SYSTEM, "Snapshot saved to %s",outfile.toLocal8Bit().constData());
                else log->Logf(GLLogStream::WARNING,"Error saving %s",outfile.toLocal8Bit().constData());
            }
            takeSnapTile=false;
            snapBuffer=QImage();
        }
    }
    update();
    glPopAttrib();
}
#endif

#if 0
setTiledView(fov, viewRatio, fAspect, nearPlane, farPlane, cameraDist);
void GLArea::setTiledView(GLdouble fovY, float viewRatio, float fAspect, GLdouble zNear, GLdouble zFar,  float cameraDist) {
    if(fovY<=5) {
        GLdouble fLeft   = -viewRatio*fAspect;
        GLdouble fRight  =  viewRatio*fAspect;
        GLdouble fBottom = -viewRatio;
        GLdouble fTop    =  viewRatio;

        GLdouble tDimX = fabs(fRight-fLeft) / totalCols;
        GLdouble tDimY = fabs(fTop-fBottom) / totalRows;


        glOrtho(fLeft   + tDimX * tileCol, fLeft   + tDimX * (tileCol+1),     /* left, right */
                fBottom + tDimY * tileRow, fBottom + tDimY * (tileRow+1),     /* bottom, top */
                cameraDist - 2.f*clipRatioNear, cameraDist+2.f*clipRatioFar);
    } else {
        GLdouble fTop    = zNear * tan(math::ToRad(fovY/2.0));
        GLdouble fRight  = fTop * fAspect;
        GLdouble fBottom = -fTop;
        GLdouble fLeft   = -fRight;

        // tile Dimension
        GLdouble tDimX = fabs(fRight-fLeft) / totalCols;
        GLdouble tDimY = fabs(fTop-fBottom) / totalRows;

        glFrustum(fLeft   + tDimX * tileCol, fLeft   + tDimX * (tileCol+1),
                  fBottom + tDimY * tileRow, fBottom + tDimY * (tileRow+1), zNear, zFar);
    }
}
#endif

#if 0
class SnapshotSetting {
public:
    QString outdir;
    QString basename;
    int counter;
    int resolution;
    bool transparentBackground;
    bool snapAllLayers;
    bool tiledSave; // if true all the tiles are saved as separated files and not joined.

    SnapshotSetting() {
        outdir=".";
        basename="snapshot";
        counter=0;
        resolution=1;
        transparentBackground=true;
        snapAllLayers=false;
        tiledSave=false;
    };
};
#endif

