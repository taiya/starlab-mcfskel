#pragma once
#include "SurfaceMeshHelper.h"
#include "LegacyLaplacianHelper.h"
#include "MatlabSurfaceMeshHelper.h"

class MatlabMeanValueLaplacianHelper : public MeanValueLaplacianHelper, public MatlabSurfaceMeshHelper{
public:
    MatlabMeanValueLaplacianHelper(SurfaceMeshModel* mesh) : SurfaceMeshHelper(mesh), LaplacianHelper(mesh), MeanValueLaplacianHelper(mesh), MatlabSurfaceMeshHelper(mesh){}

    void setConstraints(ScalarVertexProperty omega_H, ScalarVertexProperty omega_L){
        if(!omega_H || !omega_L) throw MissingPropertyException("Invalid");
        if(!vindex)  throw MissingPropertyException("v:index");
        
        /// Initialize "omega_L"
        {
            mxArray* _w = mxCreateDoubleMatrix(mesh->n_vertices(),1,mxREAL);
            double* w = mxGetPr(_w); 
            foreach(Vertex v, mesh->vertices())
                w[ vindex[v] ] = omega_L[v];
            put("omega_L", _w);
        }
        
        /// Initialize "omega_H"
        {
            mxArray* _w = mxCreateDoubleMatrix(mesh->n_vertices(),1,mxREAL);
            double* w = mxGetPr(_w); 
            foreach(Vertex v, mesh->vertices())
                w[ vindex[v] ] = omega_H[v];
            put("omega_H", _w);
        }
        
        /// Initialize x0
        {
            const char* x0_property = "v:point";
            Vector3VertexProperty points = mesh->get_vertex_property<Vector3>(x0_property);        
            if(!points) throw MissingPropertyException(x0_property);
            mxArray* _x0 = mxCreateDoubleMatrix(mesh->n_vertices(),3,mxREAL);
            double* x0 = mxGetPr(_x0);
            Index nrows = mesh->n_vertices();
            foreach(Vertex v, mesh->vertices()){
                // qDebug() << points[v];
                x0[ vindex[v] + 0*nrows ] = points[v].x();
                x0[ vindex[v] + 1*nrows ] = points[v].y();
                x0[ vindex[v] + 2*nrows ] = points[v].z();
            }
            put("x0", _x0);
        }
    }
    
    void solve(){
        // eval("[~,dir] = system('pwd')");
        // eval("save('/Users/ata2/Developer/skelcollapse/my.mat')");        
#if 0
        /// Re-weight the laplacian matrix
        eval("OMEGA_L = spdiags(omega_L, 0, nv, nv);");
        eval("L2 = L' * spdiags(omega_L,0,nv,nv) * L;");
        eval("L2 = L2 - spdiags( spdiags(L2,0), 0, nv, nv );");
        eval("L2 = L2 + spdiags( sum(L2,2), 0, nv, nv );");
                
        /// Add constraints on LHS and RHS
        eval("P = spdiags(omega_H, 0, nv, nv);");
        eval("b = P*x0;");
        eval("L2 = L2+P;");
        /// Solve
        eval("x = L2 \\ b;");
#endif
#if 1
        eval("nv = size(L,1);");
        eval("OMEGA_L = spdiags(omega_L,0,nv,nv);");
        eval("L = OMEGA_L * L';");
        eval("P = spdiags(omega_H, 0, nv, nv);");
        eval("LHS = [L; P];");
        eval("RHS = [ zeros(nv,3) ; P*x0 ];");
        // eval("save('/Users/ata2/Developer/skelcollapse/test.mat')");
                
        eval("x = LHS \\ RHS;");
        check_for_warnings();
#endif
#if 0
        eval("save('/Users/ata2/Developer/skelcollapse/matlab.mat')");        
        eval("x=x0;");
#endif
    }
    
    
};
