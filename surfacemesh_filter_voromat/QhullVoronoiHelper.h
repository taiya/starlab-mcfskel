#pragma once

#include "SurfaceMeshHelper.h"
#include "StarlabDrawArea.h"
#include <QStack>

#include "QhullError.h"
#include "QhullQh.h"
#include "QhullFacet.h"
#include "QhullFacetList.h"
#include "QhullFacetSet.h"
#include "QhullRidge.h"
#include "QhullLinkedList.h"
#include "QhullVertex.h"
#include "Qhull.h"

#include <cstdio>   /* for printf() of help message */
#include <ostream>
#include <set>

using std::cerr;
using std::cin;
using std::cout;
using std::endl;

using namespace orgQhull;

class VoronoiHelper : public SurfaceMeshHelper{

private:
    ScalarVertexProperty vangle;    /// NAN
    ScalarVertexProperty vradii;    /// NAN
    StarlabDrawArea* drawArea;      /// !NAN

	std::vector< Vector3 > loci;
	std::vector< std::vector<uint> > cells;

	std::vector<int> poleof;				// The index of the voronoi pole the vertex refers to
	std::vector< std::vector<int> > scorr; 	// The index the surface sample to which a pole corresponds to

	std::vector<double> alpha;
	std::vector<double> radii;

	int nvertices, nvornoi;

public:
    VoronoiHelper(SurfaceMeshModel* mesh, StarlabDrawArea* drawArea) : SurfaceMeshHelper(mesh){
        this->drawArea = drawArea;
        vangle = mesh->add_vertex_property<Scalar>("v:angle",0);
        vradii = mesh->add_vertex_property<Scalar>("v:radii",0);
    }

    void computeVoronoiDiagram()
    {
		// Note: we will use entire point cloud indiscriminately 
		mesh->garbage_collection();

        Qhull qhull("", 3, mesh->n_vertices(), &points.data()->x(), "v Qbb");
        //QhullFacetList facets= qhull.facetList();
        //std::cout << facets;
		
		// Extract vertices & cell structure
		cells.resize( mesh->n_vertices() );
		int i = 0;

		foreach(QhullFacet f, qhull.facetList())
		{
			if(f.isUpperDelaunay()) continue;

			QhullPoint qhpnt = f.voronoiVertex(qhull.runId());
			Vector3 p(qhpnt[0], qhpnt[1], qhpnt[2]);

            if(!mesh->bbox().contains(p)) continue;

			loci.push_back(p);
			
			foreach(QhullVertex v, f.vertices())
				cells[v.point().id()].push_back(i);
			
			i++;
			//drawArea->drawPoint(p, 10);
		}

		// DEBUG:
		//foreach(std::vector<uint> cell, cells) drawCell(cell);

		// Used later
		nvertices = this->mesh->n_vertices();
		nvornoi = loci.size();
	}

private:

	void drawCell(std::vector<uint> cell)
	{
		foreach(uint loci_id, cell) drawArea->drawPoint(loci[loci_id], 5);
	}

	struct Spoke{
		double x;
		double y;
		double z;

		Spoke(){ x=y=z=0; }
		Spoke( double x, double y, double z ){
			this->x = x;
			this->y = y;
			this->z = z;
		}

		double angle( const Spoke& s ){
			return acos( (this->x)*(s.x)+(this->y)*(s.y)+(this->z)*(s.z) );
		}
	};

public:
	void searchVoronoiPoles()
	{
		Vector3 surf_vertex;
		Vector3 voro_vertex;
		Vector3 surf_normal;

        poleof = std::vector<int>(nvertices, 0);
        scorr  = std::vector< std::vector<int> > (nvornoi, std::vector<int>(4, 0));

		//--- Keeps track of how many 
		//    surface points a voronoi loci has been 
		//    associated with. Assuming general positions 
		//    this number should be always 4.
        std::vector<int> counter(nvornoi, 0);

		//--- For every voronoi cell
        for(uint sidx = 0; (int)sidx < nvertices; sidx++)
		{
			// Retrieve surface vertex
			surf_vertex = points[Vertex(sidx)];
			surf_normal = vnormal[Vertex(sidx)];

			// Index and distance to furthest voronoi loci
			double max_neg_t = DBL_MAX;
            double max_neg_i = 0;

			// For each element of its voronoi cell
			for(int j = 0; j < (int)cells[sidx].size(); j++)
			{
				int vidx = cells[sidx][j];

				voro_vertex = loci[vidx];

				// Mark the fact that (voronoi) vidx corresponds to (surface) sidx 
				// (in the next free location and update this location)
				// the freesub-th correspondent of the voronoi vertex is vidx
				int freesub = counter[vidx];
				if( freesub < 4 ){
					counter[vidx]++;
					scorr[vidx][freesub] = sidx;
				}

				// Project the loci on the vertex normal & Retain furthest 
                double t = dot(Vector3(voro_vertex - surf_vertex), surf_normal);
				if(t < 0 && t < max_neg_t){
                    //drawArea->drawPoint(voro_vertex);
					max_neg_t = t;
					max_neg_i = vidx;
				}
			}

			// Save pole to which surface corresponds
			// Store index (possibly nan!! into buffer)
			poleof[sidx] = max_neg_i;
		}
	}    

	void getMedialSpokeAngleAndRadii()
	{
		//--- Temp data
		Vector3 surf_vertex;
		Vector3 voro_vertex;    
		Vector3 s; // temp spoke data
		double curralpha;
 
		alpha = std::vector<double> ( nvornoi );
		radii = std::vector<double> ( nvornoi );

		//--- For every voronoi vertex
		for(int vidx = 0; vidx < nvornoi; vidx++)
		{
			/// Do not use invalid poles
			//if( ispole.get(vidx) == 0.0 ) continue;

			// Retrieve vertex coordinates
			voro_vertex = loci[vidx];

			// Create the medial spokes
			// General positions => only 4 vertices / loci
            std::vector<Spoke> spokes;
			for(int i_sidx = 0; i_sidx < 4; i_sidx++)
			{
				// Retrieve surface coordinate
				int sidx = scorr[vidx][i_sidx];
				surf_vertex = points[Vertex(sidx)];

				// Create spoke
				s = surf_vertex - voro_vertex;

				// Spoke length (shouldn't this be same as we are voronoi loci?)
				radii[vidx] = s.norm();
    
				// Normalize spoke
				s.normalize();
				spokes.push_back( Spoke(s[0],s[1],s[2]) );
			}


			// Measure largest spoke aperture
			// and store it in output
			double alpha_max = 0;
			for(int i=0; i<4; i++){
				Spoke& s1 = spokes[i];
				for(int j=0; j < 4; j++){
					Spoke& s2 = spokes[j];
					curralpha = s1.angle(s2);
					if( curralpha > alpha_max ){
						alpha_max = curralpha;
					}
				}
			}
			alpha[vidx] = alpha_max;
		}
	}

    void setToMedial(bool isEmbed)
    {
		foreach(Vertex v, mesh->vertices())
		{
			int pole = poleof[v.idx()];

            // Set vertex positions to medial
            if(isEmbed)
                points[v] = loci[pole];

			// Export angle/radii from medial to surface
			vangle[v] = alpha[pole];
			vradii[v] = radii[pole];
		}

        if(!isEmbed){
            Vector3VertexProperty vpoles = mesh->add_vertex_property<Vector3>("v:pole");

            foreach(Vertex v, mesh->vertices())
                vpoles[v] = loci[ poleof[v.idx()] ];
        }
	}
};
