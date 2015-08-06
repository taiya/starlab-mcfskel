#pragma once
#define DEMEAN
#include <Eigen/Cholesky> ///< for point to plane
#include <Eigen/Geometry>
#include <QString>
#include <QDebug>
#include "StarlabDrawArea.h"

namespace RigidMotionEstimator{
    /// @param The source, one point per column
    /// @param The target, one point per column
    /// @param The confidence weights (will be normalized to sum=1)
    inline Eigen::Affine3d point_to_point(Eigen::Map<Eigen::Matrix3Xd>& X, Eigen::Matrix3Xd& Y, const Eigen::VectorXd& w) {
        Eigen::VectorXd w_normalized = w/w.sum();
        Eigen::Vector3d src_mean, dst_mean;
        for(unsigned int i=0; i<3; ++i) {
            src_mean(i) = (X.row(i).array()*w_normalized.transpose().array()).sum();
            dst_mean(i) = (Y.row(i).array()*w_normalized.transpose().array()).sum();
        }
        X.colwise() -= src_mean;
        Y.colwise() -= dst_mean;
        Eigen::Matrix3d sigma = X * w_normalized.asDiagonal() * Y.transpose();
        Eigen::JacobiSVD<Eigen::Matrix3d,Eigen::FullPivHouseholderQRPreconditioner> svd(sigma, Eigen::ComputeFullU | Eigen::ComputeFullV);
        Eigen::Affine3d transformation;
        if(svd.matrixU().determinant()*svd.matrixV().determinant()<0.0) {
            Eigen::Vector3d S = Eigen::Vector3d::Ones(); S(2) = -1.0;
            transformation.linear().noalias() = svd.matrixV()*S.asDiagonal()*svd.matrixU().transpose();
        } else {
            transformation.linear().noalias() = svd.matrixV()*svd.matrixU().transpose();
        }
        transformation.translation().noalias() = dst_mean - transformation.linear()*src_mean;
        X = transformation.linear()*X;
        X.colwise() += src_mean + transformation.translation();
        Y.colwise() += dst_mean;
        return transformation;
    }
    inline Eigen::Affine3d point_to_point(Eigen::Map<Eigen::Matrix3Xd>& X, Eigen::Matrix3Xd& Y) {
        return point_to_point(X, Y, Eigen::VectorXd::Ones(X.cols()));
    }
}

namespace RigidMotionEstimator{
    /// @param The source, one point per column
    /// @param The target, one point per column
    /// @param The target normals, one normal per column
    /// @param The confidence weights (will be normalized to sum=1)
    inline Eigen::Affine3d point_to_plane(Eigen::Map<Eigen::Matrix3Xd>& X,
                                          Eigen::Matrix3Xd& Y,
                                          Eigen::Matrix3Xd& N,
                                          const Eigen::VectorXd& w,
                                          const Eigen::VectorXd& u){
        typedef Eigen::Matrix<double, 6, 6> Matrix6d;
        typedef Eigen::Matrix<double, 6, 1> Vector6d;
        typedef Eigen::Block<Matrix6d, 3, 3> MyBlock; /// Reference chunk
        
        /// Normalize weight vector (only relative magnitudes important)
        Eigen::VectorXd w_normalized = w/w.sum();

        /// De-mean w.r.t. rotating model (unstiffens rotation)
        Eigen::Vector3d X_mean;
        { 
            for(unsigned int i=0; i<3; ++i) 
                X_mean(i) = (X.row(i).array()*w_normalized.transpose().array()).sum();
            X.colwise() -= X_mean;
            Y.colwise() -= X_mean;
        }
        
        /// Caches some computation
        Eigen::MatrixXd C = Eigen::MatrixXd::Zero(3,X.cols());
        {
            for(int i=0; i<X.cols(); i++){ 
                C.col(i) = X.col(i).cross( N.col(i) ); 
            }
        }
        
        /// Prepare LHS and RHS
        Matrix6d LHS = Matrix6d::Zero();
        Vector6d RHS = Vector6d::Zero();
        {
            // typedef Eigen::Matrix3d MyBlock; /// Reference chunk
            MyBlock TL = LHS.topLeftCorner<3,3>();
            MyBlock TR = LHS.topRightCorner<3,3>();
            MyBlock BR = LHS.bottomRightCorner<3,3>();
            #pragma omp parallel sections
            {
                #pragma omp section
                for(int i=0; i<X.cols(); i++) TL.selfadjointView<Eigen::Upper>().rankUpdate(C.col(i),w(i));
                #pragma omp section
                for(int i=0; i<X.cols(); i++) TR += ( C.col(i)*N.col(i).transpose() ) * w(i);
                #pragma omp section
                for(int i=0; i<X.cols(); i++) BR.selfadjointView<Eigen::Upper>().rankUpdate(N.col(i),w(i));
                #pragma omp section
                for(int i=0; i<C.cols(); i++){
                    double dist_to_plane = -((X.col(i) - Y.col(i)).dot(N.col(i)) - u(i)) * w(i);
                    RHS.head<3>() += C.col(i) * dist_to_plane;
                    RHS.tail<3>() += N.col(i) * dist_to_plane;
                }
            }
            LHS = LHS.selfadjointView<Eigen::Upper>();
        }

        /// Solve linear system => R,T
        Eigen::Affine3d transformation;
        {
            Eigen::LDLT<Matrix6d> ldlt(LHS);
            if(ldlt.info() != Eigen::Success) std::cout << "LDLT failed" << std::endl;
            RHS = ldlt.solve(RHS);
            transformation.translation() = RHS.tail<3>();
            transformation.linear() << 1.0, -RHS[2], +RHS[1], RHS[2],  1.0, -RHS[0], -RHS[1],  RHS[0],  1.0;
        }
        
        /// Pass matrix through SVD to ensure it is a pure rotation and that it is not mirrored
        {
            Eigen::JacobiSVD<Eigen::Matrix3d> svd(transformation.linear(), Eigen::ComputeFullU | Eigen::ComputeFullV);
            if(svd.matrixU().determinant()*svd.matrixV().determinant()<0.0) {
                Eigen::Vector3d S = Eigen::Vector3d::Ones(); S(2) = -1.0;
                transformation.linear().noalias() = svd.matrixU()*S.asDiagonal()*svd.matrixV().transpose();
            } else {
                transformation.linear().noalias() = svd.matrixU()*svd.matrixV().transpose();
            }
        }
            
        /// Apply transformation
        {
            X = transformation.linear()*X;
            X.colwise() += X_mean + transformation.translation();
            Y.colwise() += X_mean;
        }

        return transformation;
    }
    inline Eigen::Affine3d point_to_plane(Eigen::Map<Eigen::Matrix3Xd>& X,
                                          Eigen::Matrix3Xd& Yp,
                                          Eigen::Matrix3Xd& Yn,
                                          const Eigen::VectorXd& w){
        return point_to_plane(X, Yp, Yn, w, Eigen::VectorXd::Zero(X.cols()));
    }
    inline Eigen::Affine3d point_to_plane(Eigen::Map<Eigen::Matrix3Xd>& X, Eigen::Matrix3Xd& Yp, Eigen::Matrix3Xd& Yn){
        return point_to_plane(X, Yp, Yn, Eigen::VectorXd::Ones(X.cols()));
    }    
}


/* Debug code
    /// Force transformation to be rotation (perfect corresps)
    Eigen::Vector3d axis(0,0,1);
    double angle = 1 * M_PI / 180.0;
    Eigen::AngleAxisd rot( angle, axis );
    Eigen::Matrix3d m = rot.toRotationMatrix();
    qDebug() << qPrintable(toString(m));
    X = rot.toRotationMatrix()*X;
*/
