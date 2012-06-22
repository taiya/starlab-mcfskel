#pragma once
#include "MatlabLaplacianHelper.h"

/// Augments the setContraints with poles information
/// Augments the setContraints with poles information
class PoleAttractorHelper : public MatlabMeanValueLaplacianHelper{
public:
    PoleAttractorHelper(SurfaceMeshModel* mesh) : SurfaceMeshHelper(mesh), LaplacianHelper(mesh), MatlabMeanValueLaplacianHelper(mesh){}
    void setConstraints(ScalarVertexProperty omega_H, ScalarVertexProperty omega_L, ScalarVertexProperty omega_P, Vector3VertexProperty poles){
        /// Do what was already there
        MatlabMeanValueLaplacianHelper::setConstraints(omega_H,omega_L);
        if(!omega_P) throw MissingPropertyException("Invalid");
        BoolVertexProperty vissplit = mesh->get_vertex_property<bool>("v:issplit");
        
        /// Initialize omega_P (poles constraints)
        {
            mxArray* _w = mxCreateDoubleMatrix(mesh->n_vertices(),1,mxREAL);
            double* w = mxGetPr(_w); 
            foreach(Vertex v, mesh->vertices())
                w[ vindex[v] ] = omega_P[v];
            put("omega_P", _w);            
        }
        
        /// Initialize p0 (poles positions)
        {
            mxArray* _p0 = mxCreateDoubleMatrix(mesh->n_vertices(),3,mxREAL);
            double* p0 = mxGetPr(_p0);
            Index nrows = mesh->n_vertices();
            foreach(Vertex v, mesh->vertices()){
                if(vissplit[v]) continue;
                p0[ vindex[v] + 0*nrows ] = poles[v].x();
                p0[ vindex[v] + 1*nrows ] = poles[v].y();
                p0[ vindex[v] + 2*nrows ] = poles[v].z();
            }
            put("p0", _p0);
        }
    }
    void solve(){
        eval("save('/Users/ata2/Developer/skelcollapse/poles.mat')");        
        eval("nv = size(L,1);");
        eval("OMEGA_L = spdiags(omega_L,0,nv,nv);");
        eval("L = OMEGA_L * L';");
        eval("H = spdiags(omega_H, 0, nv, nv);");
        eval("P = spdiags(omega_P, 0, nv, nv);");
        eval("LHS = [L; H; P];");
        eval("RHS = [ zeros(nv,3) ; H*x0 ; P*p0];");
        eval("x = LHS \\ RHS;");
        check_for_warnings();
    }
}; 
