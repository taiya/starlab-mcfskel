//=============================================================================
// Copyright (C) 2001-2005 by Computer Graphics Group, RWTH Aachen
// Copyright (C) 2011 by Graphics & Geometry Group, Bielefeld University
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public License
// as published by the Free Software Foundation, version 2.
//
// This library is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================


//== INCLUDES =================================================================

#include "IO.h"
#include <stdio.h>

//== IMPLEMENTATION ===========================================================

typedef Surface_mesh::Point Point;
typedef Surface_mesh::Normal Normal;

// helper function
template <typename T> void read(FILE* in, T& t)
{
    int err = 0;
    err = fread(&t, 1, sizeof(t), in);
}


//-----------------------------------------------------------------------------


bool read_off(Surface_mesh& mesh, const std::string& filename)
{
    int err = 0;
    unsigned int       i, j, idx;
    unsigned int       nV, nF, nE;
    Eigen::Vector3d    p, n;
    Eigen::Vector2d    t;
    Surface_mesh::Vertex  v;
    char               line[100], *c;
    bool               has_texcoords = false;
    bool               has_normals   = false;
    bool               has_colors    = false;
    bool               has_hcoords   = false;
    bool               has_dim       = false;
    bool               is_binary     = false;


    // if mesh is not empty we need an offset for vertex indices
    const unsigned int voffset = mesh.n_vertices();


    // open file (in ASCII mode)
    FILE* in = fopen(filename.c_str(), "r");
    if (!in) return false;


    // read header: [ST][C][N][4][n]OFF BINARY
    c = fgets(line, 100, in);
    assert(c != NULL);
    c = line;
    if (c[0] == 'S' && c[1] == 'T') { has_texcoords = true; c += 2; }
    if (c[0] == 'C') { has_colors  = true; ++c; }
    if (c[0] == 'N') { has_normals = true; ++c; }
    if (c[0] == '4') { has_hcoords = true; ++c; }
    if (c[0] == 'n') { has_dim     = true; ++c; }
    if (strncmp(c, "OFF", 3) != 0) { fclose(in); return false; } // no OFF
    if (strncmp(c+4, "BINARY", 6) == 0) is_binary = true;
    const bool binary = is_binary; // const might speed things up


    // colors, homogeneous coords, and vertex dimension != 3 are not supported
    if (has_colors || has_hcoords || has_dim)
    {
        fclose(in);
        return false;
    }


    // properties
    Surface_mesh::Vertex_property<Surface_mesh::Normal>  normals;
    Surface_mesh::Vertex_property<Surface_mesh::Texture_coordinate>  texcoords;
    if (has_normals)   normals   = mesh.vertex_property<Surface_mesh::Normal>("v:normal");
    if (has_texcoords) texcoords = mesh.vertex_property<Surface_mesh::Texture_coordinate>("v:texcoord");


    // if binary: reopen file in binary mode
    if (binary)
    {
        fclose(in);
        in = fopen(filename.c_str(), "rb");
        c = fgets(line, 100, in);
        assert(c != NULL);
    }

	// (ennetws) bug fix for comments, empty lines..
	while(!binary && true && !feof(in)){
		fgets(line, 100, in);
		if(strlen(line) > 4 && line[0] != '#')
			break;
	}


	// #Vertice, #Faces, #Edges
	if (binary)
	{
		read(in, nV);
		read(in, nF);
		read(in, nE);
	}
	else
	{
		err = sscanf(line, "%d %d %d", (int*)&nV, (int*)&nF, (int*)&nE); // (ennetws) bug fix
	}
	mesh.reserve(nV, std::max(3*nV, nE), nF);

    // read vertices: pos [norma] [texcoord]
    if (has_normals && has_texcoords)
    {
        for (i=0; i<nV && !feof(in); ++i)
        {
            if (binary)
            {
                read(in, p);
                read(in, n);
                read(in, t);
            }
            else
            {
                err = fscanf(in, "%lf %lf %lf %lf %lf %lf %lf %lf", &p[0], &p[1], &p[2], &n[0], &n[1], &n[2], &t[0], &t[1]);
            }
            v = mesh.add_vertex(p);
            normals[v] = n;
            texcoords[v][0] = t[0];
            texcoords[v][1] = t[1];
        }
    }
    else if (has_normals)
    {
        for (i=0; i<nV && !feof(in); ++i)
        {
            if (binary)
            {
                read(in, p);
                read(in, n);
            }
            else
            {
                err = fscanf(in, "%lf %lf %lf %lf %lf %lf", &p[0], &p[1], &p[2], &n[0], &n[1], &n[2]);
            }
            v = mesh.add_vertex(p);
            normals[v] = n;
        }
    }
    else if (has_texcoords)
    {
        for (i=0; i<nV && !feof(in); ++i)
        {
            if (binary)
            {
                read(in, p);
                read(in, t);
            }
            else
            {
                err = fscanf(in, "%lf %lf %lf %lf %lf", &p[0], &p[1], &p[2], &t[0], &t[1]);
            }
            v = mesh.add_vertex(p);
            texcoords[v][0] = t[0];
            texcoords[v][1] = t[1];
        }
    }
    else
    {
        for (i=0; i<nV && !feof(in); ++i)
        {
            if (binary)
            {
                read(in, p);
            }
            else
            {
                err = fscanf(in, "%lf %lf %lf", &p[0], &p[1], &p[2]);
            }
            mesh.add_vertex(p);
        }
    }



    // read faces: #N v[1] v[2] ... v[n-1]
    std::vector<Surface_mesh::Vertex> vertices;
    for (i=0; i<nF; ++i)
    {
        if (binary)
        {
            read(in, nV);
            vertices.resize(nV);
            for (j=0; j<nV; ++j)
            {
                read(in, idx);
                vertices[j] = Surface_mesh::Vertex(idx+voffset);
            }
        }
        else
        {
            err = fscanf(in, "%d", (int*)&nV);
            vertices.resize(nV);
            for (j=0; j<nV; ++j)
            {
                err = fscanf(in, "%d", (int*)&idx);
                vertices[j] = Surface_mesh::Vertex(idx+voffset);
            }
        }
        mesh.add_face(vertices);
    }



    // File was successfully parsed.
    fclose(in);
    return true;
}


//-----------------------------------------------------------------------------


bool write_off(const Surface_mesh& mesh, const std::string& filename)
{
    FILE* out = fopen(filename.c_str(), "w");
    if (!out)
        return false;


    // header
    fprintf(out, "OFF\n%d %d 0\n", mesh.n_vertices(), mesh.n_faces());


    // vertices
    Surface_mesh::Vertex_property<Surface_mesh::Point> points = mesh.get_vertex_property<Point>("v:point");
    for (Surface_mesh::Vertex_iterator vit=mesh.vertices_begin(); vit!=mesh.vertices_end(); ++vit)
    {
        const Point& p = points[vit];
        fprintf(out, "%.10f %.10f %.10f\n", p[0], p[1], p[2]);
    }


    // faces
    for (Surface_mesh::Face_iterator fit=mesh.faces_begin(); fit!=mesh.faces_end(); ++fit)
    {
        int nV = mesh.valence(fit);
        fprintf(out, "%d", nV);
        Surface_mesh::Vertex_around_face_circulator fvit=mesh.vertices(fit), fvend=fvit;
        do
        {
            fprintf(out, " %d", ((Surface_mesh::Vertex)fvit).idx());
        }
        while (++fvit != fvend);
        fprintf(out, "\n");
    }

    fclose(out);
    return true;
}


//=============================================================================
