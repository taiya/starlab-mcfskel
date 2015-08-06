#include <QMouseEvent>
#include "surfacemesh_mode_info.h"
#include "StarlabDrawArea.h"
#include "FindDialog.h"
#include "float.h"
#include <qgl.h>
#include "SurfaceMeshNormalsHelper.h"
#include "FaceBarycenterHelper.h"
#include "SurfaceMeshModel.h"

using namespace qglviewer;
using namespace SurfaceMesh;

// Fast OpenGL text
#include "font.inl"

void surfacemesh_mode_info::create()
{
    fontImage = QImage(":/images/font.png");

    update();
}

bool surfacemesh_mode_info::documentChanged()
{
    update();

    return true;
}

void surfacemesh_mode_info::update()
{
    // to do: mesh needs to be consistent with selected mesh
    points = mesh()->vertex_coordinates();

	SurfaceMeshHelper h(mesh());
    faceAreas = h.computeFaceAreas();
    elengs = h.computeEdgeLengths();

    SurfaceMesh::NormalsHelper nh(mesh());
    faceNormals = nh.compute_face_normals();

    SurfaceMesh::FaceBarycenterHelper fh(mesh());
    faceCenters = fh.compute();

	visualize = QVector<bool>(HDGE_IDX+1, true);
	visualize[HDGE_IDX] = false; // Hide half-edges by default

	drawArea()->setSelectRegionWidth(50);
	drawArea()->setSelectRegionHeight(50);

	selectedIdx = -1;
}

void surfacemesh_mode_info::drawWithNames()
{
    int i = -1;

	Vec viewDir = drawArea()->camera()->viewDirection().unit();
	Vector3 cameraNormal(viewDir[0],viewDir[1],viewDir[2]);

	double vt = 0;

	glPointSize(5.0f);

	// Vertices
    foreach(const Vertex vit, mesh()->vertices()){
		i = i + 1;

		Halfedge he = mesh()->halfedge(vit);
		if(he.is_valid())
		{
			Face f = mesh()->face(he);
			if(mesh()->is_valid(f) && dot(faceNormals[f], cameraNormal) > vt) continue;
		}

		Vector3 v = points[vit];
        glPushName(i);
        glBegin(GL_POINTS);
            glVertex3d(v.x(), v.y(), v.z());
        glEnd();
        glPopName();
    }

	// Faces
	foreach(const Face f, mesh()->faces()){
		++i;

		if(mesh()->is_valid(f) && dot(faceNormals[f], cameraNormal) > vt) continue;
		glPushName(i);
		glBegin(GL_POINTS);
        Vector3 v = faceCenters[f];
        glVertex3d(v.x(), v.y(), v.z());
		glEnd();
		glPopName();
	}

	// Edges
	foreach(const Edge e, mesh()->edges()){
		++i;

		Face f = mesh()->face(mesh()->halfedge(e,0));
		if(mesh()->is_valid(f) && dot(faceNormals[f], cameraNormal) > vt) continue;

		glPushName(i);
		glBegin(GL_POINTS);
        Vector3 p1 = points[mesh()->vertex(e,0)];
        Vector3 p2 = points[mesh()->vertex(e,1)];
        Vector3 v = (p1 + p2) * 0.5;
		glVertex3d(v.x(), v.y(), v.z());
		glEnd();
		glPopName();
	}
}

int surfacemesh_mode_info::correctIndex(int i)
{
    int nv = mesh()->n_vertices();
    int nf = mesh()->n_faces();
    int ne = mesh()->n_edges();

    // Find actual index
    if(i < nv)
        selectedType = VERT_IDX;
    else if(i < nv + nf){
        selectedType = FACE_IDX;
        i -= nv;
    }
    else if(i < nv + nf + ne){
        selectedType = EDGE_IDX;
        i -= (nv + nf);
    }

    return i;
}

bool surfacemesh_mode_info::postSelection(const QPoint& p)
{
	Q_UNUSED(p);

    selectedIdx = correctIndex( drawArea()->selectedName() );

    return true;
}

bool surfacemesh_mode_info::endSelection( const QPoint& p )
{
    Q_UNUSED(p);
    return false;
}

void surfacemesh_mode_info::decorate()
{
    if(document()->isBusy()) return;

    // Bound check on selected
    if(selectedType == VERT_IDX) qMin(selectedIdx, (int)mesh()->n_vertices());
    if(selectedType == FACE_IDX) qMin(selectedIdx, (int)mesh()->n_faces());
    if(selectedType == EDGE_IDX) qMin(selectedIdx, (int)mesh()->n_edges());
    if(selectedType == HDGE_IDX) qMin(selectedIdx, (int)mesh()->n_halfedges());

	if(selectedIdx < 0)
	{
		// Draw visible visualizations on entire mesh
		if(visualize[VERT_IDX]) drawIndex(VERT_IDX, QColor(255,0,0));
		if(visualize[FACE_IDX]) drawIndex(FACE_IDX, QColor(0,255,0));
		if(visualize[EDGE_IDX]) drawIndex(EDGE_IDX, QColor(0,0,255));
		if(visualize[HDGE_IDX]) drawIndex(HDGE_IDX, QColor(255,127,0));

		// Draw instructions string
		glColor4d(1,1,1,1);
		beginDrawIndex();

		QString info = QString("Vertices %1\t Faces %2\n").arg(mesh()->n_vertices()).arg(mesh()->n_faces());
		info += "Press SHIFT and click item for info\n";

		drawStringQuad(50,50,qPrintable(info),true);
		endDrawIndex();
	}
	else
	{
		drawSelectedItem();
		drawItemInfo();
	}
}

void surfacemesh_mode_info::drawIndex(DrawElementType indexType, QColor color, double vt)
{
	drawArea()->qglColor(color);

	Vec viewDir = drawArea()->camera()->viewDirection().unit();
	Vector3 cameraNormal(viewDir[0],viewDir[1],viewDir[2]);

	beginDrawIndex();

	switch (indexType)
	{
	case VERT_IDX:
		foreach(const Vertex v, mesh()->vertices())
		{
            if(mesh()->valence(v))
            {
                Face f = mesh()->face(mesh()->halfedge(v));
                if(mesh()->is_valid(f) && dot(faceNormals[f], cameraNormal) > vt) continue;
            }

			drawIndexVertex(v);
		}
		break;

	case FACE_IDX:
		foreach(const Face f, mesh()->faces()){
			if(mesh()->is_valid(f) && dot(faceNormals[f], cameraNormal) > vt) continue;
			drawIndexFace(f);
		}
		break;

	case EDGE_IDX:
		foreach(const Edge e, mesh()->edges()){
			Face f = mesh()->face(mesh()->halfedge(e,0));
			if(mesh()->is_valid(f) && dot(faceNormals[f], cameraNormal) > vt) continue;

            Vector3 p1 = points[mesh()->vertex(e,0)];
            Vector3 p2 = points[mesh()->vertex(e,1)];
            Vector3 c = (p1 + p2) * 0.5;

			drawIndexEdge(e.idx(), c);
		}
		break;

	case HDGE_IDX:
		foreach(const Halfedge h, mesh()->halfedges()){
			Face f = mesh()->face(h);
			if(mesh()->is_valid(f) && dot(faceNormals[f], cameraNormal) > vt) continue;

            Vector3 p1 = points[mesh()->from_vertex(h)];
            Vector3 p2 = points[mesh()->to_vertex(h)];
            Vector3 c = (p1 + p2 + faceCenters[f]) / 3.0;

			drawIndexEdge(h.idx(), c);
		}
		break;
	}

	endDrawIndex();
}

void surfacemesh_mode_info::beginDrawIndex()
{
	glGetDoublev(GL_CURRENT_COLOR,font_color);

	drawArea()->startScreenCoordinatesSystem();

    initFont(fontImage);

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, fontTexture);

	glBegin(GL_QUADS);
}

void surfacemesh_mode_info::endDrawIndex()
{
	glEnd();

	glDisable(GL_TEXTURE_2D);
	glEnable(GL_LIGHTING);
	glEnable(GL_DEPTH_TEST);

	drawArea()->stopScreenCoordinatesSystem();
}

void surfacemesh_mode_info::drawIndexVertex( Vertex v, bool isShadow )
{
    Vector3 p = points[v];
    Vec proj = cameraProjection(p);
	sprintf(gl_text_buf,"%d", v.idx());
	drawStringQuad(proj.x - (stringWidth(gl_text_buf) * 0.5), proj.y, gl_text_buf, isShadow);
}

void surfacemesh_mode_info::drawIndexFace( Face f, bool isShadow )
{
    Vector3 c = faceCenters[f];
	Vec proj = cameraProjection(c);
	sprintf(gl_text_buf,"%d", f.idx());
	drawStringQuad(proj.x - (stringWidth(gl_text_buf) * 0.5), proj.y, gl_text_buf, isShadow);
}

void surfacemesh_mode_info::drawIndexEdge( int i, Vector3 c, bool isShadow )
{
	Vec proj = cameraProjection(c);
	sprintf(gl_text_buf,"%d", i);
	drawStringQuad(proj.x - (stringWidth(gl_text_buf) * 0.5), proj.y, gl_text_buf, isShadow);
}

void surfacemesh_mode_info::drawSelectedItem()
{
	glDisable(GL_LIGHTING);

	switch(selectedType){
	case VERT_IDX:
		{
			Vertex v(selectedIdx);
			Vec proj = cameraProjection(points[v]);

			// Draw spokes
            if(mesh()->valence(v))
            {
                Surface_mesh::Halfedge_around_vertex_circulator adjE(mesh(), v), eend = adjE;
                glColor4d(0,1,0,1);
                glLineWidth(3.0);
                glDisable(GL_DEPTH_TEST);
                glBegin(GL_LINES);
                do {
                    Edge e = mesh()->edge(adjE);
                    Vector3 p1 = points[mesh()->vertex(e,0)];
                    Vector3 p2 = points[mesh()->vertex(e,1)];
                    glVertex3d(p1.x(), p1.y(), p1.z());
                    glVertex3d(p2.x(), p2.y(), p2.z());
                } while(++adjE != eend);
                glEnd();
                glEnable(GL_DEPTH_TEST);
            }

			// Draw circle
			double radius = 12;
			glColor3d(1,1,1);
			drawArea()->startScreenCoordinatesSystem();
			glBegin(GL_TRIANGLE_FAN);
				for (double angle = 0; angle < M_PI * 2; angle += M_PI/12.0)
					glVertex2f(proj.x + sin(angle) * radius, proj.y + cos(angle) * radius);
			glEnd();
			drawArea()->stopScreenCoordinatesSystem();

			// Draw numerical indices
			beginDrawIndex();

			// Draw vertex index
			glColor4d(1,0.2,0.2,1);
			drawIndexVertex(v);

            if(mesh()->valence(v))
            {
                // Draw adjacent vertices
                glColor4d(1,1,1,1);
                Surface_mesh::Vertex_around_vertex_circulator adjV(mesh(), v), vend = adjV;
                do { drawIndexVertex(adjV); } while(++adjV != vend);

                // Draw adjacent faces
                glColor4d(0,1,0.5,1);
                Surface_mesh::Face_around_vertex_circulator adjF(mesh(), v), fend = adjF;
                do { drawIndexFace(adjF); } while(++adjF != fend);

                // Draw adjacent edges
                glColor4d(0,0,0.6,1);
                Surface_mesh::Halfedge_around_vertex_circulator adjE(mesh(), v), eend = adjE;
                do {
                    Edge e = mesh()->edge(adjE);
                    Vector3 p1 = points[mesh()->vertex(e,0)];
                    Vector3 p2 = points[mesh()->vertex(e,1)];
                    Vector3 c = (p1 + p2) * 0.5;
                    drawIndexEdge(e.idx(), c);
                } while(++adjE != eend);
            }

			endDrawIndex();
			glDisable(GL_LIGHTING);
		}
		break;

	case FACE_IDX:
		{
			Face f(selectedIdx);

			glDisable(GL_LIGHTING);
			glDisable(GL_DEPTH_TEST);

			// Draw solid triangle
			glColor4d(0,0.8,0,0.5);
			glBegin(GL_POLYGON);
				Surface_mesh::Vertex_around_face_circulator vit, vend;
				vit = vend = mesh()->vertices(f);
                do{ glVertex3dv(points[vit].data()); } while(++vit != vend);
			glEnd();

			// Draw face edges
			glColor4d(0,1,0,1);
			glLineWidth(3.0f);
			glBegin(GL_LINE_LOOP);
				vit = vend = mesh()->vertices(f);
                do{ glVertex3dv(points[vit].data()); } while(++vit != vend);
			glEnd();
			glEnable(GL_DEPTH_TEST);

			beginDrawIndex();

			// Draw face index
			glColor4d(0.2,1,0.2,1);
			drawIndexFace(f);

			// Draw adjacent vertices
			glColor4d(1,0,0,1);
			Surface_mesh::Vertex_around_face_circulator adjV(mesh(), f);
			do { drawIndexVertex(adjV); } while(++adjV != vend);

			// Draw adjacent edges
			glColor4d(0,0,1,1);
			Surface_mesh::Halfedge_around_face_circulator adjE(mesh(), f), eend = adjE;
			do { 
				Edge e = mesh()->edge(adjE);
                Vector3 p1 = points[mesh()->vertex(e,0)];
                Vector3 p2 = points[mesh()->vertex(e,1)];
                Vector3 c = (p1 + p2) * 0.5;
				drawIndexEdge(e.idx(), c); 
			} while(++adjE != eend);

			endDrawIndex();

		}
		break;
        
    case HDGE_IDX:
        break;
        
	case EDGE_IDX:
		{
			Edge e(selectedIdx);

			glDisable(GL_LIGHTING);
			glDisable(GL_DEPTH_TEST);

			Face f1 = mesh()->face(mesh()->halfedge(e,0));
			Face f2 = mesh()->face(mesh()->halfedge(e,1));

			// Draw two solid triangles
			glColor4d(0,0.8,0,0.5);
			glBegin(GL_POLYGON);
			Surface_mesh::Vertex_around_face_circulator vit, vend;
			vit = vend = mesh()->vertices(f1);
            do{ glVertex3dv(points[vit].data()); } while(++vit != vend);
			glEnd();
			
			if(mesh()->is_valid(f2)){
				glBegin(GL_POLYGON);
				vit = vend = mesh()->vertices(f2);
                do{ glVertex3dv(points[vit].data()); } while(++vit != vend);
				glEnd();
			}

			// Draw selected edge
			glColor4d(0.8,0.8,1,1);
			glLineWidth(3.0f);
			glBegin(GL_LINES);
            glVertex3dv(points[mesh()->vertex(e,0)].data());
            glVertex3dv(points[mesh()->vertex(e,1)].data());
			glEnd();

			beginDrawIndex();

			// Draw edge index
            Vector3 p1 = points[mesh()->vertex(e,0)];
            Vector3 p2 = points[mesh()->vertex(e,1)];
            Vector3 c = (p1 + p2) * 0.5;
			glColor4d(0,0,1,1);
			drawIndexEdge(e.idx(), c);

			// Draw adjacent faces indices
			glColor4d(0.2,1,0.2,1);
			drawIndexFace(f1);
			if(mesh()->is_valid(f2)) drawIndexFace(f2);

			// Draw vertices
			glColor4d(1,0,0,1);
			drawIndexVertex(mesh()->vertex(e,0));
			drawIndexVertex(mesh()->vertex(e,1));

			endDrawIndex();

			glEnable(GL_LIGHTING);
		}
		break;
	}

	glEnable(GL_LIGHTING);
}

void surfacemesh_mode_info::drawItemInfo()
{
	QString log;

	switch(selectedType){
    case HDGE_IDX: break;
    case VERT_IDX:
		{
			// Position
			Vertex v(selectedIdx);

			log += QString("Vertex (%1)\n").arg(v.idx());
			log += QString("\tValence    %1\n").arg(mesh()->valence(v));
            log += QString("\tBoundry    %1\n").arg(mesh()->is_boundary(v));
			
			// Properties
			foreach(std::string pname, mesh()->vertex_properties()){
				QString property_name = pname.c_str();
				// Bool properties
				Surface_mesh::Vertex_property<bool> boolprop = mesh()->get_vertex_property<bool>(pname);
				if(boolprop.is_valid()) log += QString("\n(%1)\t%2").arg(property_name).arg(boolprop[v]);

				// Integer properties
				Surface_mesh::Vertex_property<int> integer = mesh()->get_vertex_property<int>(pname);
				if(integer.is_valid()) log += QString("\n(%1)\t%2").arg(property_name).arg(integer[v]);

				// Double properties
				Surface_mesh::Vertex_property<double> dbl = mesh()->get_vertex_property<double>(pname);
				if(dbl.is_valid()) log += QString("\n(%1)\t%2").arg(property_name).arg(dbl[v]);

                // Vector properties
                Surface_mesh::Vertex_property<Vector3> vctr = mesh()->get_vertex_property<Vector3>(pname);
                if(vctr.is_valid()) log += QString("\n(%1)\t%2\t%3\t%4").arg(property_name).arg(vctr[v][0]).arg(vctr[v][1]).arg(vctr[v][2]);
			}
		}
		break;
	case FACE_IDX:
		{
			Face f(selectedIdx);

			// Center & normal
            QString center = QString("%1\t%2\t%3").arg(faceCenters[f][0]).arg(faceCenters[f][1]).arg(faceCenters[f][2]);

			// Collect points
			QVector<Vector3> pnts; 
			Surface_mesh::Vertex_around_face_circulator vit = mesh()->vertices(f),vend=vit;
			do{ pnts.push_back(points[vit]); } while(++vit != vend);

			// Compute angle stats of face
			double minAngle(DBL_MAX), maxAngle(-DBL_MAX);
			Surface_mesh::Halfedge_around_face_circulator h(mesh(), f), eend = h;
			do{ 
				Vector3 a = points[mesh()->to_vertex(h)];
				Vector3 b = points[mesh()->from_vertex(h)];
				Vector3 c = points[mesh()->to_vertex(mesh()->next_halfedge(h))];

				double angle = acos(dot((b-a).normalized(),(c-a).normalized()));

				minAngle = qMin(angle, minAngle);
				maxAngle = qMax(angle, maxAngle);
			} while(++h != eend);

			log += QString("Face (%1)\n").arg(f.idx());
			log += QString("\tValence    %1\n").arg(mesh()->valence(f));
			log += QString("\tBoundry    %1\n").arg(mesh()->is_boundary(f));
			log += QString("\tArea       %1\n").arg(faceAreas[f]);
            log += QString("\tCenter     %1\n").arg(center);
			log += QString("\tAngle      %1\n").arg(QString("min  %1\tmax  %2").arg(DEGREES(minAngle),3).arg(DEGREES(maxAngle),3));

			// Properties
			foreach(std::string pname, mesh()->face_properties()){
				QString property_name = pname.c_str();
				// Bool properties
				Surface_mesh::Face_property<bool> boolprop = mesh()->get_face_property<bool>(pname);
				if(boolprop.is_valid()) log += QString("\n(%1)\t%2").arg(property_name).arg(boolprop[f]);

				// Integer properties
				Surface_mesh::Face_property<int> integer = mesh()->get_face_property<int>(pname);
				if(integer.is_valid()) log += QString("\n(%1)\t%2").arg(property_name).arg(integer[f]);

				// Double properties
				Surface_mesh::Face_property<double> dbl = mesh()->get_face_property<double>(pname);
				if(dbl.is_valid()) log += QString("\n(%1)\t%2").arg(property_name).arg(dbl[f]);

                // Vector properties
                Surface_mesh::Face_property<Vector3> vctr = mesh()->get_face_property<Vector3>(pname);
                if(vctr.is_valid()) log += QString("\n(%1)\t%2\t%3\t%4").arg(property_name).arg(vctr[f][0]).arg(vctr[f][1]).arg(vctr[f][2]);
			}
		}
		break;
	case EDGE_IDX:
		{
			Edge e(selectedIdx);
			
			// Dihedral angle
			double diangle = 0, sign = 1.0;
			Face f1 = mesh()->face(mesh()->halfedge(e,0));
			Face f2 = mesh()->face(mesh()->halfedge(e,1));
			if(mesh()->is_valid(f2)){
				diangle = acos(dot(faceNormals[f1], faceNormals[f2]));
				
				// Get opposing vertices
				Vector3 pa = points[mesh()->to_vertex(mesh()->next_halfedge(mesh()->halfedge(e, 0)))];
				Vector3 pb = points[mesh()->to_vertex(mesh()->next_halfedge(mesh()->halfedge(e, 1)))];

                sign = (pb-pa).dot(faceNormals[f1]);
			}

			log += QString("Edge (%1)\n").arg(e.idx());
			log += QString("\tLength          %1\n").arg(elengs[e]);
			log += QString("\tBoundry         %1\n").arg(mesh()->is_boundary(e));
			log += QString("\tDihedral angle  %1\n").arg(sign*DEGREES(diangle));

			// Properties
			foreach(std::string pname, mesh()->edge_properties()){
				QString property_name = pname.c_str();
				// Bool properties
				Surface_mesh::Edge_property<bool> boolprop = mesh()->get_edge_property<bool>(pname);
				if(boolprop.is_valid()) log += QString("\n(%1)\t%2").arg(property_name).arg(boolprop[e]);

				// Integer properties
				Surface_mesh::Edge_property<int> integer = mesh()->get_edge_property<int>(pname);
				if(integer.is_valid()) log += QString("\n(%1)\t%2").arg(property_name).arg(integer[e]);

				// Double properties
				Surface_mesh::Edge_property<double> dbl = mesh()->get_edge_property<double>(pname);
				if(dbl.is_valid()) log += QString("\n(%1)\t%2").arg(property_name).arg(dbl[e]);
			}
		}
		break;
	}

	// Draw description string
	glColor4d(1,1,1,1);
	beginDrawIndex();
	drawStringQuad(50,50,qPrintable(log),true);
	endDrawIndex();
}

qglviewer::Vec surfacemesh_mode_info::cameraProjection( Vector3 c )
{
	return drawArea()->camera()->projectedCoordinatesOf(Vec(c.x(), c.y(), c.z()));
}

bool surfacemesh_mode_info::keyPressEvent( QKeyEvent* event )
{
	bool used = false;

    // User select by number
    if(event->key() == Qt::Key_S)
    {
        FindDialog findDialog(mesh()->n_vertices(), mesh()->n_faces());
        findDialog.exec();

        selectedIdx = correctIndex( findDialog.index );

        used = true;
    }

	if(event->key() == Qt::Key_V) { 
		visualize[VERT_IDX] = !visualize[VERT_IDX]; 
		used = true;
	}

	if(event->key() == Qt::Key_F) { 
		visualize[FACE_IDX] = !visualize[FACE_IDX]; 
		used = true;
	}

	if(event->key() == Qt::Key_E) { 
		visualize[EDGE_IDX] = !visualize[EDGE_IDX]; 
		used = true;
	}

	if(event->key() == Qt::Key_H) { 
		visualize[HDGE_IDX] = !visualize[HDGE_IDX]; 
		used = true;
	}

	if(event->key() == Qt::Key_R) { 
		update();
		used = true;
	}

	drawArea()->updateGL();

	return used; 
}

bool surfacemesh_mode_info::keyReleaseEvent( QKeyEvent* event )
{
	Q_UNUSED(event);
	return false;
}

Q_EXPORT_PLUGIN(surfacemesh_mode_info)
