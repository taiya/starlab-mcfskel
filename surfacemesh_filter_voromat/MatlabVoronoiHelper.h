#pragma once
#include "MatlabSurfaceMeshHelper.h"
#include "StarlabDrawArea.h"

class MatlabVoronoiHelper : public MatlabSurfaceMeshHelper{
private:
    ScalarVertexProperty vangle; /// NAN 
    ScalarVertexProperty vradii; /// NAN
    StarlabDrawArea* drawArea; /// !NAN
private:
    struct Voronoi{
        // Vertices (loci)
        long nvertices, nvertices2;
        double* vertices;
        
        // Cells
        mxArray* cells_mxa;
        int ncells;
        
        
        
        // Constructor
        Voronoi(mxArray* vertices_mxa, mxArray* cells_mxa){
            // Init vertices
            vertices = mxGetPr( vertices_mxa );       
            nvertices = mxGetDimensions(vertices_mxa)[0];
            nvertices2 = 2*nvertices;
            
            // Init cells (need to keep track of it)
            this->cells_mxa = cells_mxa;
            ncells = mxGetDimensions(cells_mxa)[0];
        }
        
        // Accessors
        Cell getCell(int i){
            mxArray* cell_mxa = mxGetCell(cells_mxa, i);
            return Cell(cell_mxa);
        }
        void getVertex( const int i, Vector3& vertex ){
            vertex[0] = vertices[              i ];
            vertex[1] = vertices[ i +  nvertices ];
            vertex[2] = vertices[ i + nvertices2 ];
        }
    };
    struct Surface{
        // Vertices
        long nvertices, nvertices2;
        double* points;
        double* normals;
                
        // Constuctor
        Surface(const mxArray* points_mxa, const mxArray* normals_mxa){
            // Init vertices
            this->points  = mxGetPr(  points_mxa );
            this->normals = mxGetPr( normals_mxa );            
            nvertices = mxGetDimensions(points_mxa)[0];
            nvertices2 = 2*nvertices;
        }
        
        // Accessors
        void getVertex( Index i, Scalar point[3] ){
            Q_ASSERT(i<nvertices);
            point[0] = points[              i ];
            point[1] = points[ i +  nvertices ];
            point[2] = points[ i + nvertices2 ];
        }
        void getNormal( Index i, Scalar normal[3]){
            Q_ASSERT(i<nvertices);
            normal[0] = normals[              i ];
            normal[1] = normals[ i +  nvertices ];
            normal[2] = normals[ i + nvertices2 ];            
        }    
    
    };
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
    MatlabVoronoiHelper(SurfaceMeshModel* mesh, StarlabDrawArea* drawArea) : SurfaceMeshHelper(mesh), MatlabSurfaceMeshHelper(mesh){
        this->drawArea = drawArea;
        vangle = mesh->add_vertex_property<Scalar>("v:angle",NAN);
        vradii = mesh->add_vertex_property<Scalar>("v:radii",NAN);
#ifdef POST_FNORMAL_CHECK
        fnormal = mesh->get_face_property<Vector3>(FNORMAL);
        if(!fnormal) throw MissingPropertyException(FNORMAL);
#endif
    }
    void computeVoronoiDiagramOf(const char* varname){
        QString voronoicommand; 
        voronoicommand.sprintf("[loci,cells] = voronoin(%s);", varname);
        // qDebug() << voronoicommand;
        eval(voronoicommand.toStdString().c_str());
    }
    void meshNormalsToVariable(const char* varname){
        Vector3VertexProperty points = mesh->get_vertex_property<Vector3>(VNORMAL);        
        if(!vindex) throw MissingPropertyException("v:index");
        mxArray* _x0 = mxCreateDoubleMatrix(mesh->n_vertices(),3,mxREAL);
        double* x0 = mxGetPr(_x0);
        Index nrows = mesh->n_vertices();
        foreach(Vertex v, mesh->vertices()){
            x0[ vindex[v] + 0*nrows ] = points[v].x();
            x0[ vindex[v] + 1*nrows ] = points[v].y();
            x0[ vindex[v] + 2*nrows ] = points[v].z();
        }
        put(varname, _x0);
    }
    void searchVoronoiPoles(const char* poleof_varname, const char* scorr_varname){
        Vector3 surf_vertex;
        Vector3 voro_vertex;
        Vector3 surf_normal;
        
        Surface S( get("points"), get("normals") );
        Voronoi V( get("loci"), get("cells") );
        
#if 0
        for(int vidx=1; vidx<V.nvertices; vidx++){            
            V.getVertex(vidx,voro_vertex);
            drawArea->drawPoint(voro_vertex,2,Qt::green);
        }
#endif   
        //--- Keeps track of how many surface points a voronoi loci has been 
        //    associated with. Assuming general positions this number should 
        //    be always 4 for vertices with general positioning
        // vector<int> counter(V.nvertices,0);
        
        //--- The index of the voronoi pole the vertex refers to
        mxArray* poleof_mxa = mxCreateDoubleMatrix(S.nvertices,1,mxREAL); 
        IndexMatrix poleof( poleof_mxa );
        
        //--- The index the surface sample to which a pole corresponds to
        mxArray* scorr_mxa = mxCreateDoubleMatrix(V.nvertices,4,mxREAL); 
        IndexMatrix scorr( scorr_mxa );
                
        //--- Keeps track of how many 
        //    surface points a voronoi loci has been 
        //    associated with. Assuming general positions 
        //    this number should be always 4.
        vector<int> counter(V.nvertices,0);
        
        //--- For every voronoi cell
        for(Index sidx=0; sidx<S.nvertices; sidx++){
            // Retrieve surface vertex
            S.getVertex(sidx,surf_vertex);
            S.getNormal(sidx,surf_normal);
            
            // Each surface vertex is a voronoi loci
            Cell c = V.getCell(sidx);       
            
            // Index and distance to furthest voronoi loci
            double max_neg_t=+FLT_MAX;
            double max_neg_i=mxGetNaN();
                    
            // For each element of its voronoi cell
            for(int j=0; j<c.numel; j++){
                // Get index of the element
                double vidx=c.getItem(j);
                
                // ignore the infinity pole
                if(vidx==0) continue;
                
                
                // Retrieve the voronoi loci coordinate
                V.getVertex(vidx,voro_vertex);
                // if(sidx==2) qDebug() << "Aka vertex: " << voro_vertex;

                // Mark the fact that (voronoi) vidx corresponds to (surface) sidx 
                // (in the next free location and update this location)
                // the freesub-th correspondent of the voronoi vertex is vidx
                int freesub = counter[vidx];
                if( freesub>4 ){
                    drawArea->drawPoint(voro_vertex,3,Qt::red);
                    throw StarlabException("How come pole %d had more than 4 corresponding surface points?\n"
                                           "I marked the point in red", sidx);
                }
                counter[vidx]++;
                scorr.set(vidx,freesub, /*=*/ sidx);
                
                
                // Project the loci on the vertex normal & Retain furthest 
                double t = dot(voro_vertex-surf_vertex, surf_normal);
                if(t<0 && t<max_neg_t){
                    max_neg_t = t;
                    max_neg_i = vidx;
                }
            }

            // Save pole to which surface corresponds
            // Store index (possibly nan!! into buffer)
            poleof.set(sidx, max_neg_i);
            
#ifdef NEGATIVE_T_CHECK           
            if(max_neg_t>0){
                V.getVertex(max_neg_i,voro_vertex);
                qDebug() << "[WARNING] possibly erroneous pole!!";
                drawArea->drawPoint(voro_vertex,5,Qt::red);
            }
#endif
            
#ifdef POST_FNORMAL_CHECK
            Vertex vit(sidx);
            V.getVertex(max_neg_i,voro_vertex);
            Surface_mesh::Face_around_vertex_circulator fvit, fvend;
            fvit = fvend = mesh->faces(vit);
            /// Attempt to see if we find a t>0, a.k.a. the pole is on  
            /// the outsideside of one of the ring faces
            Scalar t=-FLT_MAX; ///
            do{
                Scalar projection = dot( fnormal[fvit], voro_vertex-surf_vertex );
                t = qMax(t, projection);                
            } while (++fvit != fvend);
            
            /// Display it in the drawArea
            if(t>=0){
                // drawArea->drawPoint(voro_vertex,10,Qt::red);                
            }
            if(!mesh->getBoundingBox().contains(voro_vertex)){
                // drawArea->drawPoint(voro_vertex,10,Qt::blue);   
                // show failed points
                if(t<0){
                    qDebug() << "t at that point was: " << t;
                    qDebug() << "max_neg_t: " << max_neg_t;
                    drawArea->drawPoint(surf_vertex,3,Qt::red); 
                    drawArea->drawPoint(voro_vertex,10,Qt::red); 
                    drawArea->drawSegment(surf_vertex,voro_vertex,2,Qt::red); 
                }
            }
#endif
            // qDebug() << "v[" << sidx << "] => " << max_neg_i;
            // if(sidx==2) break;
        }
        
        /// Put the variable in the matlab workspace
        put(poleof_varname,poleof_mxa);
        put(scorr_varname,scorr_mxa);
    }    
    void getMedialSpokeAngleAndRadii(const char* alpha_varname, const char* radii_varname){
        //--- Temp data
        Vector3 surf_vertex;
        Vector3 voro_vertex;    
        double s[3]; // temp spoke data
        double s_nrm; // spoke norm
        double curralpha;
        
        //--- Read input arguments
        Surface S( get("points"), get("normals") );
        Voronoi V( get("loci"), get("cells") );

        //--- Array of active poles (those to whom at least one surface element corresponds)
        eval("ispole = zeros(size(loci,1),1);");
        eval("ispole(poleof) = 1;");
        mxArray* _ispole = get("ispole");
        ScalarVector ispole(_ispole);               
                
        //--- Pole 2 Surface correspondence
        IndexMatrix V2Si(get("scorr"));
        
        //--- Output arguments initializations & wrappers
        mxArray* alpha_mxa = mxCreateDoubleMatrix(V.nvertices,1,mxREAL);
        mxArray* radii_mxa = mxCreateDoubleMatrix(V.nvertices,1,mxREAL);
        ScalarMatrix alpha( alpha_mxa );
        ScalarMatrix radii( radii_mxa );
    
        // Clear at every iteration
        vector<Spoke> spokes;
    
        //--- For every voronoi vertex
        for(int vidx=1; vidx<V.nvertices; vidx++){
            /// Do not use invalid poles
            if( ispole.get(vidx) == 0.0 ) continue;

            // Retrieve vertex coordinates
            V.getVertex(vidx,voro_vertex);         
#if 0
            if((vidx%100)==1){
                drawArea->drawPoint(voro_vertex,2,Qt::blue);
            }
#endif
            
            // Create the medial spokes
            // General positions => only 4 vertices/loci
            spokes.clear();
            for(int i_sidx=0; i_sidx<4; i_sidx++){
                // Retrieve surface coordinate
                int sidx = V2Si.get(vidx,i_sidx);
                
                /// Fixed by starting from vidx=1
                // if(sidx==-1){
                //    qDebug() << "[WARNING] " << i_sidx << "-th surface correspondent of voronoi vertex #" << vidx << "not set" << "voro_vertex" << voro_vertex;
                //    continue;
                // }
                S.getVertex(sidx,surf_vertex);
                
#if 0
                if((vidx%100)==1){ 
                    drawArea->drawPoint(surf_vertex,2,Qt::blue);
                    drawArea->drawSegment(voro_vertex, surf_vertex, 2,Qt::blue);
                }
#endif           
                // Create spoke
                s[0] = surf_vertex[0] - voro_vertex[0];
                s[1] = surf_vertex[1] - voro_vertex[1];
                s[2] = surf_vertex[2] - voro_vertex[2];
    
                // Spoke length (shouldn't this be same as we are voronoi loci?)
                s_nrm = sqrt( s[0]*s[0] + s[1]*s[1] + s[2]*s[2] );
                radii.set(vidx, /* = */s_nrm);
                      
#if 0
                if(s_nrm>2){
                    // drawArea->drawSegment(surf_vertex, voro_vertex,2,Qt::red);
                    drawArea->drawPoint(voro_vertex,2);
                    // drawArea->drawPoint(surf_vertex,2);
                }
#endif           
                // Normalize spoke
                s[0] /= s_nrm; 
                s[1] /= s_nrm; 
                s[2] /= s_nrm;
                spokes.push_back( Spoke(s[0],s[1],s[2]) );
            }
            
            
            // Measure largest spoke aperture
            // and store it in output
            double alpha_max = 0;
            for(int i=0; i<4; i++){
                Spoke& s1 = spokes[i];
                for(int j=0; j<4; j++){
                    Spoke& s2 = spokes[j];
                    curralpha = s1.angle(s2);
                    if( curralpha>alpha_max ){
                        alpha_max = curralpha;
                    }
                }
            }
            alpha.set(vidx, /* = */alpha_max);
        }  
        
        /// Put the variables in the matlab workspace
        put(radii_varname,radii_mxa);
        put(alpha_varname,alpha_mxa);
    }
};
