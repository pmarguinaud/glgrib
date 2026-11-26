#include "glGrib/Land.h"
#include "glGrib/Trigonometry.h"
#include "glGrib/OpenGL.h"
#include "glGrib/ShapeLib.h"
#include "glGrib/Options.h"
#include "glGrib/ConstrainedDelaunay.h"
#include "glGrib/Subdivide.h"

#include <iostream>
#include <omp.h>
#include <algorithm>

namespace glGrib
{

void Land::layer_t::render (const OptionsLandLayer & opts) const
{
  Program * program = Program::load ("LAND");

  program->set ("scale0", opts.scale);
  program->set ("color0", opts.color);
  program->set ("debug", opts.debug.on);
     
  glDrawElements (GL_TRIANGLES, 3 * d.numberOfTriangles, GL_UNSIGNED_INT, nullptr);
}

void Land::render (const View & view, const OptionsLight & light) const
{
  Program * program = Program::load ("LAND");
  program->use (); 

  view.setMVP (program);

  for (size_t i = 0; i < layers.size (); i++)
    if (opts.layers[i].on)
      layers[i].VAID.render (opts.layers[i]);
     
  view.delMVP (program);

}

void Land::layer_t::triangulate
    (int_v * _pos_offset, 
     int_v * _pos_length,
     int_v * _ind_offset,
     int_v * _ind_length,
     const std::vector<unsigned int> & indl,
     std::vector<float> * _lonlat,
     std::vector<unsigned int> * _ind)
{
  int_v   & pos_offset = *_pos_offset;
  int_v   & pos_length = *_pos_length;
  int_v   & ind_offset = *_ind_offset;
  int_v   & ind_length = *_ind_length;
  std::vector<float> & lonlat     = *_lonlat    ;
  std::vector<unsigned int> & ind = *_ind       ;

  // Offset/length of each ring (the closing point is not part of the ring)
  pos_offset.push_back (+0);
  pos_length.push_back (-1);

  for (size_t i = 0; i < indl.size (); i++)
    {
      if (indl[i] == OpenGL::restart)
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

  const int nr = pos_offset.size ();

  // Bigger rings first, for load balance
  int_v ord (nr);
  for (int i = 0; i < nr; i++)
    ord[i] = i;
  std::sort (ord.begin (), ord.end (), [&pos_length] (int i, int j) 
             { return pos_length[j] < pos_length[i]; });

  // Triangulate each ring ; triangles use local indices, and points added by CDT
  // (only when edges cross, i.e. invalid rings) are numbered after the ring vertices
  std::vector<std::vector<unsigned int> > tri (nr);
  std::vector<std::vector<float> > added (nr);
  int nfail = 0;

#pragma omp parallel for schedule (dynamic, 1) reduction (+:nfail)
  for (int l = 0; l < nr; l++)
    {
      int j = ord[l], len = pos_length[j];
      if (len < 3)
        continue;
      // A ring that cannot be triangulated is left empty
      if (! ConstrainedDelaunay::processRing (lonlat, pos_offset[j], len, &tri[j], &added[j]))
        nfail++;
    }

  if (nfail > 0)
    std::cerr << "Land : constrained Delaunay triangulation failed for " << nfail 
              << " ring(s) of " << nr << ", not filled" << std::endl;

  // Added points are inserted right after the vertices of their ring, so that each
  // ring keeps a contiguous range of points (this is required by Subdivide)
  size_t nadd = 0;
  for (int j = 0; j < nr; j++)
    nadd += added[j].size () / 2;

  if (nadd > 0)
    {
      std::vector<float> lonlat1;
      lonlat1.reserve (lonlat.size () + 2 * nadd);
      int_v pos_offset1 (nr);
      for (int j = 0; j < nr; j++)
        {
          int b = pos_offset[j];
          int e = j + 1 < nr ? pos_offset[j+1] : lonlat.size () / 2;
          int len = std::max (pos_length[j], 0);
          pos_offset1[j] = lonlat1.size () / 2;
          lonlat1.insert (lonlat1.end (), lonlat.begin () + 2 * b, lonlat.begin () + 2 * (b + len));
          lonlat1.insert (lonlat1.end (), added[j].begin (), added[j].end ());
          lonlat1.insert (lonlat1.end (), lonlat.begin () + 2 * (b + len), lonlat.begin () + 2 * e);
          pos_length[j] += added[j].size () / 2;
        }
      pos_offset.swap (pos_offset1);
      lonlat.swap (lonlat1);
    }

  // Offset/length for each indices block, without gaps
  ind_offset.resize (nr);
  ind_length.resize (nr);

  int ind_size = 0;
  for (int j = 0; j < nr; j++)
    {
      ind_offset[j] = ind_size;
      ind_length[j] = tri[j].size ();
      ind_size += ind_length[j];
    }

  ind.resize (ind_size);

#pragma omp parallel for
  for (int j = 0; j < nr; j++)
    for (int k = 0; k < ind_length[j]; k++)
      ind[ind_offset[j]+k] = pos_offset[j] + tri[j][k];

}

void Land::layer_t::subdivide 
   (const int_v & ind_offset,
    const int_v & ind_length,
    const int_v & pos_offset,
    const int_v & pos_length,
    std::vector<unsigned int> * _ind,
    std::vector<float> * _lonlat,
    const float angmax)
{
  std::vector<unsigned int> & ind    = *_ind;
  std::vector<float>        & lonlat = *_lonlat;

  int_v ind_offset_sub;
  int_v ind_length_sub;
  int_v pos_offset_sub;
  int_v pos_length_sub;

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
  std::vector<Subdivide> sr (n);

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
  int_v pts_offset (n);
  int_v tri_offset (n);

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


void Land::layer_t::setup (const OptionsLandLayer & opts)
{
  OptionsLines lopts;
  std::vector<float> lonlat;
  std::vector<unsigned int> indl;
  
  lopts.path = opts.path; lopts.selector = opts.selector;
  ShapeLib::read (lopts, &lonlat, &indl, lopts.selector);
  
  int_v pos_offset, pos_length;
  int_v ind_offset;
  int_v ind_length;
  std::vector<unsigned int> ind;
  
  triangulate (&pos_offset, &pos_length, &ind_offset, &ind_length,
               indl, &lonlat, &ind);
  
  
  if (opts.subdivision.on)
    {
      const float angmax = deg2rad * opts.subdivision.angle;
      subdivide (ind_offset, ind_length, pos_offset, pos_length,
                &ind, &lonlat, angmax);
    }
  
  
  
  d.numberOfTriangles = ind.size () / 3;
  
  d.vertexbuffer = OpenGLBufferPtr<float> (lonlat);
  d.elementbuffer = OpenGLBufferPtr<unsigned int> (ind);

}

void Land::setup (const OptionsLand & o)
{
  if (! o.on)
    return;

  opts = o;

  layers.resize (opts.layers.size ());

  for (size_t i = 0; i < opts.layers.size (); i++)
    if (opts.layers[i].on)
      layers[i].setup (opts.layers[i]);

  setReady ();
}

void Land::layer_t::setupVertexAttributes () const
{
  Program * program = Program::load ("LAND");
  d.vertexbuffer->bind (GL_ARRAY_BUFFER);
  auto attr = program->getAttributeLocation ("vertexLonLat");
  glEnableVertexAttribArray (attr); 
  glVertexAttribPointer (attr, 2, GL_FLOAT, GL_FALSE, 0, nullptr); 
  d.elementbuffer->bind (GL_ELEMENT_ARRAY_BUFFER);
}

}
