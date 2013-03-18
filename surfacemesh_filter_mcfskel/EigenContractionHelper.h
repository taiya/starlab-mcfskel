#pragma once

#include <iomanip>
#include <QElapsedTimer>
#include <Eigen/Core>
#include <Eigen/Sparse>
#include <Eigen/CholmodSupport>
#include "SurfaceMeshHelper.h"
#include "CotangentLaplacianHelper.h"
#include "Logfile.h"

using namespace Eigen;

class EigenContractionHelper : public SurfaceMesh::SurfaceMeshHelper{
private:
    int nrows, ncols;

    typedef Surface_mesh::Vertex_property<uint>  IndexVertexProperty;

    IndexVertexProperty vindex;   
    SparseMatrix<double> LHS;
    MatrixXd RHS;
    MatrixXd X;

public:
    EigenContractionHelper(SurfaceMeshModel* mesh) : SurfaceMeshHelper(mesh){}
    void evolve(ScalarVertexProperty omega_H, ScalarVertexProperty omega_L, ScalarVertexProperty omega_P, Vector3VertexProperty poles){
        ScalarHalfedgeProperty hweight = CotangentLaplacianHelper(mesh).computeCotangentEdgeWeights("h:weight");
        
        updateVertexIndexes();
#if 0
        createLHS(hweight,omega_L,omega_H);
        createRHS(omega_H,points);
#else
        createLHS(hweight,omega_L,omega_H,omega_P);
        createRHS(omega_H,points,omega_P,poles);        
#endif
        solveByFactorization(VPOINT);
    }
    void updateVertexIndexes();
    void createLHS(ScalarHalfedgeProperty hweight, ScalarVertexProperty omega_L, ScalarVertexProperty omega_H);    
    void createLHS(ScalarHalfedgeProperty hweight, ScalarVertexProperty omega_L, ScalarVertexProperty omega_H, ScalarVertexProperty omega_P);
    void createRHS(ScalarVertexProperty omega_H, Vector3VertexProperty vinitial);
    void createRHS(ScalarVertexProperty omega_H, Vector3VertexProperty vinitial, ScalarVertexProperty omega_P, Vector3VertexProperty poles);
    
    void solveByFactorization(std::string vsolution);
    void solve_linear_least_square(SparseMatrix<double> & A, MatrixXd & B, MatrixXd & X);
};

void EigenContractionHelper::updateVertexIndexes(){
    /// Create indexes for mesh vertices
    vindex = mesh->vertex_property<uint>("v:index",0);
    uint curr_vidx = 0;
    foreach(Vertex v, mesh->vertices())
        vindex[v] = curr_vidx++;
}

void EigenContractionHelper::createLHS(ScalarHalfedgeProperty hweight, ScalarVertexProperty omega_L, ScalarVertexProperty omega_H){
    nrows = 2*mesh->n_vertices();
    ncols = mesh->n_vertices();

    /// Allocate memory
    LHS.resize(nrows,ncols);
    RHS = MatrixXd::Zero(nrows, 3);
    X = MatrixXd::Zero(ncols, 3);

    /// Assemble sparse matrix with eigen triplets        
    typedef Triplet<double> TripletDouble;
    std::vector< TripletDouble > triplets;
    triplets.reserve(ncols * 6);
    
    /// Fill laplacian matrix (off diagonal)
    foreach(Halfedge h, mesh->halfedges()){
        Vertex v0 = mesh->from_vertex(h);
        Vertex v1 = mesh->to_vertex(h);
        triplets.push_back(TripletDouble(vindex[v0], vindex[v1], hweight[h]*omega_L[v0]));
    }
    /// Fill laplacian matrix (diagonal)
    foreach(Vertex v, mesh->vertices()){
        double sum = 0;
        foreach(Halfedge h, mesh->onering_hedges(v))
            sum += hweight[h];
        triplets.push_back(TripletDouble(vindex[v],vindex[v], -sum));
    }
    
    /// Set bottom half of matrix (constraints)
    foreach(Vertex v, mesh->vertices())
        triplets.push_back(TripletDouble(vindex[v] + ncols, vindex[v], omega_H[v]));
    
    LHS.setFromTriplets(triplets.begin(), triplets.end());
}

void EigenContractionHelper::createLHS(ScalarHalfedgeProperty hweight, ScalarVertexProperty omega_L, ScalarVertexProperty omega_H, ScalarVertexProperty omega_P){
    nrows = 3*mesh->n_vertices();
    ncols = mesh->n_vertices();

    /// Allocate memory
    LHS.resize(nrows,ncols);
    RHS = MatrixXd::Zero(nrows, 3);
    X = MatrixXd::Zero(ncols, 3);

    /// Assemble sparse matrix with eigen triplets        
    typedef Triplet<double> TripletDouble;
    std::vector< TripletDouble > triplets;
    triplets.reserve(ncols*9); /// 7 entries/row on laplacian, 1 on each constraint matrix
    
    /// Fill laplacian matrix (off diagonal)
    foreach(Halfedge h, mesh->halfedges()){
        Vertex v0 = mesh->from_vertex(h);
        Vertex v1 = mesh->to_vertex(h);
        triplets.push_back(TripletDouble(vindex[v0], vindex[v1], hweight[h]*omega_L[v0]));
    }
    /// Fill laplacian matrix (diagonal)
    foreach(Vertex v, mesh->vertices()){
        double sum = 0;
        foreach(Halfedge h, mesh->onering_hedges(v))
            sum += hweight[h];
        triplets.push_back(TripletDouble(vindex[v],vindex[v], -sum));
    }
    
    /// Set bottom half of matrix (constraints)
    foreach(Vertex v, mesh->vertices())
        triplets.push_back(TripletDouble(vindex[v] + ncols, vindex[v], omega_H[v]));
    /// Set bottom half of matrix (constraints)
    foreach(Vertex v, mesh->vertices())
        triplets.push_back(TripletDouble(vindex[v] + 2*ncols, vindex[v], omega_P[v]));
    
    
    LHS.setFromTriplets(triplets.begin(), triplets.end());
}

/// Retrieve & fill RHS (top half is zeros)
void EigenContractionHelper::createRHS(ScalarVertexProperty omega_H, Vector3VertexProperty vinitial){
    /// Mesh => constraint vectors
    // TIMER timer.start();
    {
        foreach(Vertex v, mesh->vertices()){
            Vector3 u = omega_H[v] * vinitial[v];
            RHS.row(ncols + vindex[v]) = Vector3d(u.x(), u.y(), u.z());
        }
    }
    // TIMER qDebug() << "Build RHS vector: " << timer.elapsed() << "ms";
}

void EigenContractionHelper::createRHS(ScalarVertexProperty omega_H, Vector3VertexProperty vinitial, ScalarVertexProperty omega_P, Vector3VertexProperty poles){
    /// Mesh => constraint vectors
    // TIMER timer.start();
    {
        foreach(Vertex v, mesh->vertices()){
            Vector3 u = omega_H[v] * vinitial[v];
            RHS.row(ncols + vindex[v]) = Vector3d(u.x(), u.y(), u.z());
        }
    }
    /// Mesh => constraint vectors
    {
        foreach(Vertex v, mesh->vertices()){
            Vector3 u = omega_P[v] * poles[v];
            RHS.row(2*ncols + vindex[v]) = Vector3d(u.x(), u.y(), u.z());
        }
    }
    // TIMER qDebug() << "Build RHS vector: " << timer.elapsed() << "ms";
}

void EigenContractionHelper::solveByFactorization(std::string vsolution){
    /// Factorize & Solve
    // TIMER timer.start();
    {
        solve_linear_least_square(LHS, RHS, X);
    }
    // TIMER qDebug() << "Factor & Solve: " << timer.elapsed() << "ms";
    
    /// Store solution in mesh property
    // TIMER timer.start();
    {
        Vector3VertexProperty _vsolution = getVector3VertexProperty(vsolution);
        foreach(Vertex v, mesh->vertices()){
            Vector3d p = X.row(vindex[v]);
            _vsolution[v] = Vector3(p[0], p[1], p[2]);
        }
    }
}

void EigenContractionHelper::solve_linear_least_square(SparseMatrix<double> & A, MatrixXd & B, MatrixXd & X){
    /// Factorize the matrix
    // tic(" CholFactor");
        SparseMatrix<double> At  = A.transpose();
        SparseMatrix<double> AtA = At * A;
        typedef CholmodDecomposition< SparseMatrix<double> > Solver;
        //typedef SimplicialLDLT< SparseMatrix<double> > Solver;
        Solver solver;
        solver.compute(AtA);
    // toc();
    
    /// 3x Solves
    // tic(" Back-Substitution");
        X.col(0) = solver.solve( At * B.col(0) );
        X.col(1) = solver.solve( At * B.col(1) );
        X.col(2) = solver.solve( At * B.col(2) );
    // toc();
}
