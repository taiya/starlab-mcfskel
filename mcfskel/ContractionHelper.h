#pragma once
#include <Eigen/Core>
#include <Eigen/Sparse>
using namespace Eigen;

#include "LaplacianHelper.h"

/// Augments the setContraints with poles information
class ContractionHelper : public MeanValueLaplacianHelper{

private:
	SparseMatrix<double> L;
	
public:
	ContractionHelper(SurfaceMeshModel* mesh) : SurfaceMeshHelper(mesh), LaplacianHelper(mesh), MeanValueLaplacianHelper(mesh){}

	void createLaplacianMatrix()
	{
		if(!hweight) throw MissingPropertyException("h:weight");

		/// Init sparse matrix
		Size nv = mesh->n_vertices();
		L = SparseMatrix<double>(nv, nv);

		/// Fill sparse matrix   
		foreach(Vertex vi, mesh->vertices())
		{
			int i = vi.idx();

			Scalar diagel = 0;
			foreach(Halfedge h, mesh->onering_hedges(vi))
				diagel += hweight[h];
	
			// diagonal
			L.insert(i, i) = -diagel;

			/// off-diagonal elements
			foreach(Halfedge h, mesh->onering_hedges(vi))
			{
				int j = mesh->to_vertex(h).idx();

				L.insert(i, j) = hweight[h];
			}
		}
	}

	void solve(ScalarVertexProperty /*omega_H*/, ScalarVertexProperty /*omega_L*/, ScalarVertexProperty /*omega_P*/, Vector3VertexProperty /*poles*/){
        qDebug() << "SOLVER MISSING";
        

		/*eval("save('poles.mat')");
		eval("nv = size(L,1);");
		eval("OMEGA_L = spdiags(omega_L,0,nv,nv);");
		eval("L = OMEGA_L * L';");
		eval("H = spdiags(omega_H, 0, nv, nv);");
		eval("P = spdiags(omega_P, 0, nv, nv);");
		eval("LHS = [L; H; P];");
		eval("RHS = [ zeros(nv,3) ; H*x0 ; P*p0];");
		eval("x = LHS \\ RHS;");*/
	}
};
