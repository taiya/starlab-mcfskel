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


bool read_obj(Surface_mesh& mesh, const std::string& filename)
{
  char   s[200];
  float  x, y, z;
  std::vector<Surface_mesh::Vertex>  vertices;


  // open file (in ASCII mode)
  FILE* in = fopen(filename.c_str(), "r");
  if (!in) return false;


  // if mesh is not empty we need an offset for vertex indices
  // also take into accout that OBJ indices start at 1 (not 0)
  const int voffset = mesh.n_vertices() - 1;


  // clear line once
  memset(&s, 0, 200);


  // parse line by line (currently only supports vertex positions & faces
  while(in && !feof(in) && fgets(s, 200, in))
  {
    // comment
    if (s[0] == '#' || isspace(s[0])) continue;

    // vertex
    else if (strncmp(s, "v ", 2) == 0)
    {
      if (sscanf(s, "v %f %f %f", &x, &y, &z))
      {
        mesh.add_vertex(Surface_mesh::Point(x,y,z));
      }
    }

    // face
    else if (strncmp(s, "f ", 2) == 0)
    {
      int component(0), nV(0);
      bool endOfVertex(false);
      char *p0, *p1(s+1);

      vertices.clear();

      // skip white-spaces
      while (*p1==' ') ++p1;

      while (p1)
      {
        p0 = p1;

        // overwrite next separator

        // skip '/', '\n', ' ', '\0', '\r' <-- don't forget Windows
        while (*p1!='/' && *p1!='\r' && *p1!='\n' && *p1!=' ' && *p1!='\0') ++p1;

        // detect end of vertex
        if (*p1 != '/')
        {
          endOfVertex = true;
        }

        // replace separator by '\0'
        if (*p1 != '\0')
        {
          *p1 = '\0';
          p1++; // point to next token
        }

        // detect end of line and break
        if (*p1 == '\0' || *p1 == '\n')
        {
          p1 = 0;
        }

        // read next vertex component
        if (*p0 != '\0')
        {
          switch (component)
          {
            case 0: // vertex
              vertices.push_back( Surface_mesh::Vertex(atoi(p0) + voffset) );
              break;

            case 1: // texture coord
              break;

            case 2: // normal
              break;
          }
        }

        ++component;

        if (endOfVertex)
        {
          component = 0;
          nV++;
          endOfVertex = false;
        }
      }

      mesh.add_face(vertices);
    }


    // clear line
    memset(&s, 0, 200);
  }


  fclose(in);
  return true;
}


//=============================================================================
