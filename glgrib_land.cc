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
  std::vector<int> pos_offset = {+0}, pos_length = {-1};

  for (int i = 0; i < indl.size (); i++)
    {
      if (indl[i] == 0xffffffff)
        {
          if (i < indl.size ()-1)
            {
              pos_offset.push_back (indl[i+1]);
              pos_length.push_back (-1);
	    }
	}
      else
        {
          pos_length.back ()++;
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
  for (int i = 0; i < pos_length.size (); i++)
    fprintf (fp, "%8d | %8d %8d\n", i, pos_offset[i], pos_length[i]);
  fclose (fp);
  }

//for (int j = 0; j < pos_offset.size (); j++)
  for (int j = 14850; j < 14860; j++)
  {
  char f[64];
  sprintf (f, "r.%3.3d.dat", j);
  FILE * fp = fopen (f, "w");
  fprintf (fp, " pos_offset, pos_length = %d, %d\n", pos_offset[j], pos_length[j]);
  for (int i = pos_offset[j]; i < pos_offset[j]+pos_length[j]+1; i++)
    fprintf (fp, " %8d | %12.2f %12.2f\n", i-pos_offset[j], rad2deg * lonlat[2*i+0], rad2deg * lonlat[2*i+1]);
  fclose (fp);
  }
//exit (0);
#endif

  // Sort rings (bigger first)
  std::vector<int> ord (pos_length.size ());

  for (int i = 0; i < pos_length.size (); i++)
    ord[i] = i;

  std::sort (ord.begin (), ord.end (), [&pos_length] (int i, int j) 
             { return pos_length[j] < pos_length[i]; });


  // Offset/length for each indices block 
  std::vector<int> ind_offset (ord.size ());
  std::vector<int> ind_length (ord.size ());

  int ind_size = 0;
  for (int k = 0; k < ord.size (); k++)
    {
      if (pos_length[k] > 2)
        {
          ind_length[k] = 3 * (pos_length[k] - 2);
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
      if (pos_length[j] < 300)
        break;
      if (pos_length[j] > 2)
        glgrib_earcut::processRing (lonlat, pos_offset[j], pos_length[j], 
                                    ind_offset[j], &ind_length[j],
                                    &ind, true);
    }


  // Process small blocks in parallel
#pragma omp parallel for
  for (int l = k; l < ord.size (); l++)
    {
      int j = ord[l];
      if (pos_length[j] > 2)
        glgrib_earcut::processRing (lonlat, pos_offset[j], pos_length[j], 
                                    ind_offset[j], &ind_length[j],
                                    &ind, false);
    }


  const float angmax = deg2rad * 1.0f;


  if (1)
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
    printf (" total = %d\n", ind_offset.back () + ind_length.back ());
  }

  std::vector<int> ind_offset_sub = ind_offset;
  std::vector<int> ind_length_sub = ind_length;
  std::vector<int> pos_offset_sub = pos_offset;
  std::vector<int> pos_length_sub = pos_length;

  if (1)
    {
      const int n = ord.size ();
      std::vector<glgrib_subdivide> sr (n);

#pragma omp parallel for
      for (int k = 0; k < n; k++)
        {
          int j = ord[k];
          sr[k].init (lonlat, ind, 
                      pos_offset_sub[j], pos_length_sub[j], 
                      ind_offset_sub[j], ind_length_sub[j]);
          sr[k].subdivide (angmax);
        }

      std::vector<int> pts_offset (n);
      std::vector<int> tri_offset (n);

      pts_offset[0] = lonlat.size () / 2;
      tri_offset[0] = ind.size ();

      for (int k = 1; k < n; k++)
        {
          pts_offset[k] = pts_offset[k-1] + sr[k-1].getPtsLength ();
          tri_offset[k] = tri_offset[k-1] + sr[k-1].getTriLength ();
        }

      lonlat.resize (lonlat.size () + 2 * (pts_offset.back () + sr.back ().getPtsLength ()));
      ind.resize (ind.size () + tri_offset.back () + sr.back ().getTriLength ());

#pragma omp parallel for
      for (int k = 0; k < n; k++)
        sr[k].append (lonlat, ind, pts_offset[k], tri_offset[k]);
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

