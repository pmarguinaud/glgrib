#include "glgrib_land.h"
#include "glgrib_opengl.h"
#include "glgrib_shapelib.h"
#include "glgrib_options.h"
#include "glgrib_earcut.h"
#include "glgrib_subdivide.h"

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <omp.h>

#include <glm/gtc/type_ptr.hpp>



void glgrib_land::render (const glgrib_view & view, const glgrib_options_light & light) const
{
  glgrib_program * program = glgrib_program::load (glgrib_program::LAND);
  program->use (); 

  view.setMVP (program);

//glDisable (GL_CULL_FACE);

  glBindVertexArray (VertexArrayID);
  glDrawElements (GL_TRIANGLES, 3 * numberOfTriangles, GL_UNSIGNED_INT, NULL);
  glBindVertexArray (0);

//glEnable (GL_CULL_FACE);

  view.delMVP (program);

}

void glgrib_land::clear ()
{
  if (isReady ())
    glDeleteVertexArrays (1, &VertexArrayID);
  glgrib_object::clear ();
}

static const float rad2deg = 180.0f / M_PI;
static const float deg2rad = M_PI / 180.0f;

void glgrib_land::setup (const glgrib_options_land & o)
{
  opts = o;

  int numberOfPoints;
  glgrib_options_lines lopts;
  unsigned int numberOfLines; 
  std::vector<float> lonlat;
  std::vector<unsigned int> indl;
  
  lopts.path = opts.path; lopts.selector = opts.selector;
  glgrib_shapelib::read (lopts, &numberOfPoints, &numberOfLines, &lonlat, 
                         &indl, lopts.selector);



  // Offset/length of each ring
  std::vector<int> offset = {+0}, length = {-1};

  for (int i = 0; i < indl.size (); i++)
    {
      if (indl[i] == 0xffffffff)
        {
          if (i < indl.size ()-1)
            {
              offset.push_back (indl[i+1]);
              length.push_back (-1);
	    }
	}
      else
        {
          length.back ()++;
	}
    }

  

#ifdef UNDEF
  {
  FILE * fp = fopen ("coords.dat", "w");
  for (int i = 0; i < indl.size (); i++)
    fprintf (fp, "%8d | %32u\n", i, indl[i]);
  fclose (fp);
  }
  {
  FILE * fp = fopen ("pos.dat", "w");
  for (int i = 0; i < length.size (); i++)
    fprintf (fp, "%8d | %8d %8d\n", i, offset[i], length[i]);
  fclose (fp);
  }

//for (int j = 0; j < offset.size (); j++)
  for (int j = 14850; j < 14860; j++)
  {
  char f[64];
  sprintf (f, "r.%3.3d.dat", j);
  FILE * fp = fopen (f, "w");
  fprintf (fp, " offset, length = %d, %d\n", offset[j], length[j]);
  for (int i = offset[j]; i < offset[j]+length[j]+1; i++)
    fprintf (fp, " %8d | %12.2f %12.2f\n", i-offset[j], rad2deg * lonlat[2*i+0], rad2deg * lonlat[2*i+1]);
  fclose (fp);
  }
//exit (0);
#endif

  // Sort rings (bigger first)
  std::vector<int> ord (length.size ());

  for (int i = 0; i < length.size (); i++)
    ord[i] = i;

  std::sort (ord.begin (), ord.end (), [&length] (int i, int j) { return length[j] < length[i]; });


  // Offset/length for each indices block 
  std::vector<int> ind_offset (ord.size ());
  std::vector<int> ind_length (ord.size ());

  int ind_size = 0;
  for (int k = 0; k < ord.size (); k++)
    {
      if (length[k] > 2)
        {
          ind_length[k] = 3 * (length[k] - 2);
          if (k > 0)
            ind_offset[k] = ind_offset[k-1] + ind_length[k-1];
	  else
            ind_offset[0] = 0;
          ind_size += ind_length[k];
        }
    }

  std::vector<unsigned int> ind (ind_size);

  // Process big blocks serially, with OpenMP on inner loops
  int k = 0;
  for (k = 0; k < ord.size (); k++)
    {
      int j = ord[k];
      if (length[j] < 300)
        break;
      if (length[j] > 2)
        glgrib_earcut::processRing (lonlat, offset[j], length[j], 
                                    ind_offset[j], &ind_length[j],
                                    &ind, true);
    }


  // Process small blocks in parallel
#pragma omp parallel for
  for (int l = k; l < ord.size (); l++)
    {
      int j = ord[l];
      if (length[j] > 2)
        glgrib_earcut::processRing (lonlat, offset[j], length[j], 
                                    ind_offset[j], &ind_length[j],
                                    &ind, false);
    }


  const float angmax = deg2rad * 1.0f;


  if (0)
  {
    FILE * fp = fopen ("ind.txt", "w");
    for (int i = 0; i < ind_offset.size (); i++)
      {
        fprintf (fp, " %8d %8d", ind_offset[i], ind_length[i]);
	if ((i > 0) && (ind_offset[i-1] + ind_length[i-1] != ind_offset[i]))
          fprintf (fp, " X");
	fprintf (fp, "\n");
      }
    fclose (fp);
  }

  if (1)
    {
      std::vector<glgrib_subdivide> sr (ord.size ());

#pragma omp parallel for
      for (int k = 0; k < ord.size (); k++)
        {
          int j = ord[k];
          sr[k].init (lonlat, ind, offset[j], length[j], 
                      ind_offset[j], ind_length[j]);
          sr[k].subdivide (angmax);
        }

      std::vector<int> points_offset    (ord.size ());
      std::vector<int> triangles_offset (ord.size ());

      points_offset   [0] = lonlat.size () / 2;
      triangles_offset[0] = ind.size ();

      for (int k = 1; k < ord.size (); k++)
        {
          points_offset   [k] = points_offset   [k-1] + sr[k-1].getPointsLength    ();
          triangles_offset[k] = triangles_offset[k-1] + sr[k-1].getTrianglesLength ();
        }

      lonlat.resize (lonlat.size () + 2 * (points_offset.back () + sr.back ().getPointsLength ()));
      ind.resize (ind.size () + triangles_offset.back () + sr.back ().getTrianglesLength ());

#pragma omp parallel for
      for (int k = 0; k < ord.size (); k++)
        sr[k].append (lonlat, ind, points_offset[k], triangles_offset[k]);
    }

  numberOfTriangles = ind.size () / 3;

  vertexbuffer = new_glgrib_opengl_buffer_ptr (lonlat.size () * sizeof (lonlat[0]), lonlat.data ());
  elementbuffer = new_glgrib_opengl_buffer_ptr (ind.size () * sizeof (ind[0]), ind.data ());

  glGenVertexArrays (1, &VertexArrayID);
  glBindVertexArray (VertexArrayID);

  vertexbuffer->bind (GL_ARRAY_BUFFER);
  glEnableVertexAttribArray (0); 
  glVertexAttribPointer (0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
    
  elementbuffer->bind (GL_ELEMENT_ARRAY_BUFFER);
  glBindVertexArray (0); 

  setReady ();
}

