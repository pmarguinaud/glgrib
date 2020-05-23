#include "glGribLand.h"
#include "glGribTrigonometry.h"
#include "glGribOpenGL.h"
#include "glGribShapeLib.h"
#include "glGribOptions.h"
#include "glGribEarCut.h"
#include "glGribSubdivide.h"

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <omp.h>

glGrib::Land & glGrib::Land::operator=(const glGrib::Land & other)
{
  clear (); 
  if ((this != &other) && other.isReady ()) 
    {   
      opts = other.opts;
      layers = other.layers;
      setupVertexAttributes (); 
      setReady (); 
    }   
  return *this;
}

void glGrib::Land::layer_t::render (const glGrib::OptionsLandLayer & opts) const
{
  glGrib::Program * program = glGrib::Program::load ("LAND");

  program->set ("scale0", opts.scale);
  program->set ("color0", opts.color);
  program->set ("debug", opts.debug.on);
     
  VAID.bind ();
  glDrawElements (GL_TRIANGLES, 3 * numberOfTriangles, GL_UNSIGNED_INT, nullptr);
  VAID.unbind ();
}

void glGrib::Land::render (const glGrib::View & view, const glGrib::OptionsLight & light) const
{
  glGrib::Program * program = glGrib::Program::load ("LAND");
  program->use (); 

  view.setMVP (program);

  for (size_t i = 0; i < layers.size (); i++)
    if (opts.layers[i].on)
      layers[i].render (opts.layers[i]);
     
  view.delMVP (program);

}

void glGrib::Land::clear ()
{
  if (isReady ())
    for (auto l : layers)
      l.clear ();
  layers.clear ();
  glGrib::Object::clear ();
}

void glGrib::Land::layer_t::triangulate 
    (std::vector<int> * _pos_offset, 
     std::vector<int> * _pos_length,
     std::vector<int> * _ind_offset,
     std::vector<int> * _ind_length,
     const std::vector<unsigned int> & indl,
     std::vector<float> * _lonlat,
     std::vector<int> * _ord,
     std::vector<unsigned int> * _ind)
{
  std::vector<int>   & pos_offset = *_pos_offset;
  std::vector<int>   & pos_length = *_pos_length;
  std::vector<int>   & ind_offset = *_ind_offset;
  std::vector<int>   & ind_length = *_ind_length;
  std::vector<float> & lonlat     = *_lonlat    ;
  std::vector<int>   & ord        = *_ord       ;
  std::vector<unsigned int> & ind = *_ind       ;

  // Offset/length of each ring
  pos_offset.push_back (+0);
  pos_length.push_back (-1);

  for (size_t i = 0; i < indl.size (); i++)
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
  ord.resize (pos_length.size ());

  for (size_t i = 0; i < pos_length.size (); i++)
    ord[i] = i;

  std::sort (ord.begin (), ord.end (), [&pos_length] (int i, int j) 
             { return pos_length[j] < pos_length[i]; });


  // Offset/length for each indices block 
  ind_offset.resize (ord.size ());
  ind_length.resize (ord.size ());

  int ind_size = 0;
  for (size_t k = 0; k < ord.size (); k++)
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

  ind.resize (ind_size);

  // Process big blocks serially, with OpenMP on inner loops
  size_t k = 0;
  for (k = 0; k < ord.size (); k++)
    {
      int j = ord[k];
      if (pos_length[j] < 300)
        break;
      if (pos_length[j] > 2)
        glGrib::EarCut::processRing (lonlat, pos_offset[j], pos_length[j], 
                                    ind_offset[j], &ind_length[j],
                                    &ind, true);
    }


  // Process small blocks in parallel
#pragma omp parallel for
  for (size_t l = k; l < ord.size (); l++)
    {
      int j = ord[l];
      if (pos_length[j] > 2)
        glGrib::EarCut::processRing (lonlat, pos_offset[j], pos_length[j], 
                                    ind_offset[j], &ind_length[j],
                                    &ind, false);
    }
}

void glGrib::Land::layer_t::subdivide 
   (const std::vector<int> & ind_offset,
    const std::vector<int> & ind_length,
    const std::vector<int> & pos_offset,
    const std::vector<int> & pos_length,
    std::vector<unsigned int> * _ind,
    std::vector<float> * _lonlat,
    const float angmax)
{
  std::vector<unsigned int> & ind    = *_ind;
  std::vector<float>        & lonlat = *_lonlat;

  std::vector<int> ind_offset_sub;
  std::vector<int> ind_length_sub;
  std::vector<int> pos_offset_sub;
  std::vector<int> pos_length_sub;

  if (0)
    {
      ind_offset_sub = ind_offset;
      ind_length_sub = ind_length;
      pos_offset_sub = pos_offset;
      pos_length_sub = pos_length;
    }
  else
    {
      // Try to reduce the number of blocks we have to process
      // by packing together smaller blocks
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
              ind_offset_sub.push_back (b ? ind_offset[i] : ind_offset_sub.back () + ind_length_sub.back ());
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
    }

  const int n = pos_offset_sub.size ();
  std::vector<glGrib::Subdivide> sr (n);

  // Subdivide information kept in subdivide objects
#pragma omp parallel for
  for (int k = 0; k < n; k++)
    {
      sr[k].init (lonlat, ind, 
                  pos_offset_sub[k], pos_length_sub[k], 
                  ind_offset_sub[k], ind_length_sub[k]);
      sr[k].subdivide (angmax);
    }

  // Count the number of triangles & plan merging
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

  // Merge indices vectors & coordinates
#pragma omp parallel for
  for (int k = 0; k < n; k++)
    sr[k].append (lonlat, ind, pts_offset[k], tri_offset[k]);

}


void glGrib::Land::layer_t::setup (const glGrib::OptionsLandLayer & opts)
{
  int numberOfPoints;
  glGrib::OptionsLines lopts;
  unsigned int numberOfLines; 
  std::vector<float> lonlat;
  std::vector<unsigned int> indl;
  
  lopts.path = opts.path; lopts.selector = opts.selector;
  glGrib::ShapeLib::read (lopts, &numberOfPoints, &numberOfLines, &lonlat, 
                         &indl, lopts.selector);
  
  std::vector<int> pos_offset, pos_length;
  std::vector<int> ord;
  std::vector<int> ind_offset;
  std::vector<int> ind_length;
  std::vector<unsigned int> ind;
  
  triangulate (&pos_offset, &pos_length, &ind_offset, &ind_length,
               indl, &lonlat, &ord, &ind);
  
  
  if (opts.subdivision.on)
    {
      const float angmax = deg2rad * opts.subdivision.angle;
      subdivide (ind_offset, ind_length, pos_offset, pos_length,
                &ind, &lonlat, angmax);
    }
  
  
  
  numberOfTriangles = ind.size () / 3;
  
  vertexbuffer = newGlgribOpenGLBufferPtr 
                      (
                        lonlat.size () * sizeof (lonlat[0]), 
                        lonlat.data ()
                      );
  elementbuffer = newGlgribOpenGLBufferPtr 
                       (
                         ind.size () * sizeof (ind[0]), 
                         ind.data ()
                       );

}

void glGrib::Land::setup (const glGrib::OptionsLand & o)
{
  opts = o;

  layers.resize (opts.layers.size ());

  for (size_t i = 0; i < opts.layers.size (); i++)
    if (opts.layers[i].on)
      layers[i].setup (opts.layers[i]);

  setupVertexAttributes ();

  setReady ();
}

void glGrib::Land::layer_t::setupVertexAttributes () const
{
  VAID.setup ();
  VAID.bind ();
  
  vertexbuffer->bind (GL_ARRAY_BUFFER);
  glEnableVertexAttribArray (0); 
  glVertexAttribPointer (0, 2, GL_FLOAT, GL_FALSE, 0, nullptr); 
  
  elementbuffer->bind (GL_ELEMENT_ARRAY_BUFFER);
  
  VAID.unbind ();
}

void glGrib::Land::setupVertexAttributes () const
{
  for (size_t i = 0; i < layers.size (); i++)
    if (opts.layers[i].on)
      layers[i].setupVertexAttributes ();
}


