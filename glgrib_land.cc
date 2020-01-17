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

  float scale0[3] = {opts.scale, opts.scale, opts.scale};
  program->set3fv ("scale0", scale0);

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

//exit (0);

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
    fp = fopen ("pos.txt", "w");
    for (int i = 0; i < pos_offset.size (); i++)
      {
        fprintf (fp, " %8d %8d", pos_offset[i], pos_offset[i]+pos_length[i]);
	if ((i > 0) && (pos_offset[i-1] + pos_length[i-1] != pos_offset[i]))
          fprintf (fp, " X");
	fprintf (fp, "\n");
      }
    fclose (fp);
  }

  if (0)
    {
  std::vector<int> ind_offset_sub;
  std::vector<int> ind_length_sub;
  std::vector<int> pos_offset_sub;
  std::vector<int> pos_length_sub;

  if(0){
  ind_offset_sub = ind_offset;
  ind_length_sub = ind_length;
  pos_offset_sub = pos_offset;
  pos_length_sub = pos_length;
  }else{
    int count = 100;
    int ntri = (ind_offset.back () + ind_length.back ()) / 3;

    int jtri = ntri / count;
    if (jtri * count != ntri)
      count++;

    ind_offset_sub.push_back (0); ind_length_sub.push_back (ind_length[0]);
    pos_offset_sub.push_back (0); pos_length_sub.push_back (pos_offset[1]-pos_offset[0]);

    const int n = ind_length.size ();
    for (int i = 1; i < n; i++)
      {
	int sz = (ind_length_sub.back () + ind_length[i]) - jtri * 3;
	bool b = (i != n-1) && (ind_offset[i+1]-ind_offset[i] != ind_length[i]);
	if (b)
          {
	    ind_offset_sub.push_back (ind_offset_sub.back () + ind_length_sub.back ());
	    ind_length_sub.push_back (ind_length[i]);
	    pos_offset_sub.push_back (pos_offset[i]);
	    pos_length_sub.push_back (pos_length[i]);
	    i++;
	    sz = 1;
	  }
	if (sz > 0)
          {
	    ind_offset_sub.push_back ( b ? ind_offset[i] : ind_offset_sub.back () + ind_length_sub.back ());
	    ind_length_sub.push_back (0);
	    ind_length_sub.back () += i+1 < n
                        ? ind_offset[i+1]-ind_offset[i+0] : ind_length[i];
	    pos_offset_sub.push_back (pos_offset[i]);
	    pos_length_sub.push_back (0);
	    pos_length_sub.back () += i+1 < n 
                        ? pos_offset[i+1]-pos_offset[i+0] : pos_length[i];
	  }
	else
          {
            ind_length_sub.back () += ind_length[i];
	    pos_length_sub.back () += i+1 < n 
                        ? pos_offset[i+1]-pos_offset[i+0] : pos_length[i];
	  }
      }
if(0)
{
  FILE * fp = fopen ("sub.txt", "w");
    for (int i = 0; i < ind_length_sub.size (); i++)
      {
      int indr1 = ind_offset_sub[i];
      int indr2 = ind_offset_sub[i]+ind_length_sub[i];
      int indp1 = pos_offset_sub[i];
      int indp2 = pos_offset_sub[i]+pos_length_sub[i];
      int pmin = *std::min_element (ind.begin () + indr1, ind.begin () + indr2);
      int pmax = *std::max_element (ind.begin () + indr1, ind.begin () + indr2);
     
      fprintf (fp, " %8d %8d %8d %8d | %8d %8d | %d %d\n", 
              indr1, indr2, indp1, indp2, pmin, pmax,
	      (pmin < indp1) || (pmin >= indp2), (pmax < indp1) || (pmax >= indp2));
      }
  fclose (fp);
}
//  exit (0);
  }

      const int n = pos_offset_sub.size ();
      std::vector<glgrib_subdivide> sr (n);

#pragma omp parallel for
      for (int k = 0; k < n; k++)
        {
if (264335 == pos_offset_sub[k])
  printf (" k = %d\n", k);
          sr[k].init (lonlat, ind, 
                      pos_offset_sub[k], pos_length_sub[k], 
                      ind_offset_sub[k], ind_length_sub[k]);
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

