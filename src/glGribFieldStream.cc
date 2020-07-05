#include "glGribFieldStream.h"
#include "glGribProgram.h"
#include "glGribPalette.h"
#include "glGribClear.h"

#include <iostream>
#include <algorithm>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

namespace glGrib
{

namespace
{

double getTime ()
{
  struct timeval tv;
  struct timezone tz;
  gettimeofday (&tv, &tz);
  return double (tv.tv_sec) + 0.000001 * double (tv.tv_usec);
}

}

FieldStream * FieldStream::clone () const
{
  FieldStream * fld = new FieldStream (Field::Privatizer ());
  *fld = *this;
  return fld;
}

void FieldStream::streamline_t::setupVertexAttributes () const
{
  d.vertexbuffer->bind (GL_ARRAY_BUFFER);
  
  for (int j = 0; j < 3; j++)
    {
      auto attr = d.vertexLonLat_attr[j];
      glEnableVertexAttribArray (attr);
      glVertexAttribPointer (attr, 2, GL_FLOAT, GL_FALSE, 0, 
                             (const void *)(j * 2 * sizeof (float)));
      glVertexAttribDivisor (attr, 1);
    }
  
  d.normalbuffer->bind (GL_ARRAY_BUFFER);
  
  for (int j = 0; j < 2; j++)
    {
      auto attr = d.norm_attr[j];
      glEnableVertexAttribArray (attr);
      glVertexAttribPointer (attr, 1, GL_FLOAT, GL_FALSE, 0, 
                             (const void *)(j * sizeof (float)));
      glVertexAttribDivisor (attr, 1);
    }
  
  d.distancebuffer->bind (GL_ARRAY_BUFFER);
  
  for (int j = 0; j < 2; j++)
    {
      auto attr = d.dist_attr[j];
      glEnableVertexAttribArray (attr); 
      glVertexAttribPointer (attr, 1, GL_FLOAT, GL_FALSE, 0, 
                             (const void *)(j * sizeof (float))); 
      glVertexAttribDivisor (attr, 1);
    }
  
}

void FieldStream::streamline_t::setup 
  (const streamline_data_t & stream_data, GLint _vertexLonLat_attr[], 
   GLint _norm_attr[], GLint _dist_attr[])
{
  for (int j = 0; j < 3; j++)
    d.vertexLonLat_attr[j] = _vertexLonLat_attr[j];
  for (int j = 0; j < 2; j++)
    d.norm_attr[j] = _norm_attr[j];
  for (int j = 0; j < 2; j++)
    d.dist_attr[j] = _dist_attr[j];

  d.vertexbuffer   = OpenGLBufferPtr<float> (stream_data.lonlat);
  d.normalbuffer   = OpenGLBufferPtr<float> (stream_data.values);
  d.distancebuffer = OpenGLBufferPtr<float> (stream_data.length);
  d.size = stream_data.size () - 1;
}

void FieldStream::setup (const Field::Privatizer, Loader * ld, const OptionsField & o, float slot)
{
  opts = o;

  d.time0 = getTime ();

  FieldMetadata meta_u, meta_v;
  FieldMetadata meta_n, meta_d;

  BufferPtr<float> data_u, data_v;
  ld->load (&data_u, opts.path, opts.geometry, slot, &meta_u, 2, 0);
  ld->load (&data_v, opts.path, opts.geometry, slot, &meta_v, 2, 1);

  setGeometry (Geometry::load (ld, opts.path[int (2 * slot)], opts.geometry));

  BufferPtr<float> data_n, data_d;

  const auto & geometry = getGeometry ();

  Loader::uv2nd (geometry, data_u, data_v, data_n, data_d, meta_u, meta_v, meta_n, meta_d);

  palette = Palette (opts.palette, 0.0f, meta_n.valmax);

  std::vector<streamline_data_t> stream_data;

  int N = 16;

  stream_data.resize (N);
  d.stream.resize (N);

  std::vector<int> it;

  const int nt = geometry->getNumberOfTriangles ();

  const int np = static_cast<int> (std::sqrt (geometry->getNumberOfPoints ()));
  int level = static_cast<int> (opts.stream.density * np / 40.0f);

  {
    BufferPtr<unsigned char> sample (nt);
    for (int i = 0; i < nt; i++)
      sample[i] = 0;

    geometry->sampleTriangle (sample, 1, level);

    for (int i = 0; i < nt; i++)
      if (sample[i])
        it.push_back (i);
  }


#pragma omp parallel for
  for (int j = 0; j < N; j++)
    {
      int i0 = ((j + 0) * it.size ()) / N;
      int i1 = ((j + 1) * it.size ()) / N;
      for (int i = i0; i < i1; i++)
        computeStreamLine (it[i], data_u, data_v, &stream_data[j], geometry);
    }

  Program * program = Program::load ("STREAM");

  GLint vertexLonLat_attr[3];
  GLint norm_attr[2];
  GLint dist_attr[2];

  for (int j = 0; j < 3; j++)
    vertexLonLat_attr[j] = program->getAttributeLocation (std::string ("vertexLonLat")
                                                        + std::to_string (j));
  for (int j = 0; j < 2; j++)
    norm_attr[j] = program->getAttributeLocation (std::string ("norm")
                                                        + std::to_string (j));
  for (int j = 0; j < 2; j++)
    dist_attr[j] = program->getAttributeLocation (std::string ("dist")
                                                        + std::to_string (j));

  for (int i = 0; i < N; i++)
    d.stream[i].setup (stream_data[i], vertexLonLat_attr, norm_attr, dist_attr);

  meta.push_back (meta_n);
  meta.push_back (meta_d);

  if (opts.no_value_pointer.on)
    {
      clear (data_n);
      clear (data_d);
    }

  values.push_back (data_n);
  values.push_back (data_d);

  setReady ();
}

float lineLineIntersect (const glm::vec2 & P1, const glm::vec2 & V1,
                         const glm::vec2 & P,  const glm::vec2 & Q)
{
  const float inf = std::numeric_limits<float>::infinity ();

  float b = V1.x * (P.y - Q.y ) - V1.y * (P.x - Q.x ); 
  float a = V1.y * (Q.x - P1.x) - V1.x * (Q.y - P1.y);

  if (std::abs (b) < 1e-6)
    return inf;

  float lambda = a / b;

  glm::vec2 M = P * lambda + Q * (1.0f - lambda);

  if (glm::dot (M - P1, V1) < 0.0f)
    return inf;

  return lambda;
}


void FieldStream::getFirstPoint 
  (int it, const BufferPtr<float> & ru, const BufferPtr<float> & rv, 
   glm::vec2 & M, glm::vec2 & Vp, glm::vec2 & Vm,
   std::valarray<float> & wp, std::valarray<float> & wm, 
   int & itp, int & itm, const const_GeometryPtr & geometry)
{
  // First point of the list; see which segment to start with, initialize weights, V and M
  int jglo[3], itri[3];

  std::valarray<float> w (3);
  glm::vec2 P[3];

  w[0] = w[1] = w[2] = 0.0f;

  geometry->getTriangleNeighbours (it, jglo, itri, P);

  // Find best edge to start with
  int i0 = std::numeric_limits<int>::max ();
  float s0 = std::numeric_limits<float>::max ();
  for (int i = 0; i < 3; i++)
    {
      int j = (i + 1) % 3;
      int k = (i + 2) % 3;
      w[i] = w[j] = 0.5f; w[k] = 0.0f;
      glm::vec2 V = glm::vec2 (w[0] * ru[jglo[0]] + w[1] * ru[jglo[1]] + w[2] * ru[jglo[2]],
                               w[0] * rv[jglo[0]] + w[1] * rv[jglo[1]] + w[2] * rv[jglo[2]]) / w.sum ();
      V = glm::normalize (V);
      glm::vec2 U = glm::normalize (P[j] - P[i]);
      float s = glm::dot (U, V);
      if (std::abs (s) < std::abs (s0))
        {
          s0 = s;
          i0 = i;
        }
    }

  
  int j0 = (i0 + 1) % 3;
  int k0 = (i0 + 2) % 3;
  
  w[i0] = w[j0] = 0.5f; w[k0] = 0.0f;
  
  // Starting point, starting vector
  M = (w[0] * P[0] + w[1] * P[1] + w[2] * P[2]) / w.sum ();

  glm::vec2 V = glm::vec2 (w[0] * ru[jglo[0]] + w[1] * ru[jglo[1]] + w[2] * ru[jglo[2]],
                           w[0] * rv[jglo[0]] + w[1] * rv[jglo[1]] + w[2] * rv[jglo[2]]) 
	                 / w.sum ();

  Vp = V; itp = it; wp = w;

  // Opposite triangle
  if ((itm = itri[i0]) >= 0)
    {
      int jglom[3], itrim[3];
      glm::vec3 xyzm[3];
      geometry->getTriangleNeighbours (itm, jglom, itrim, xyzm);

      wm[0] = wm[1] = wm[2] = 0.0f;

      for (int i = 0; i < 3; i++)
      for (int im = 0; im < 3; im++)
        if (jglom[im] == jglo[i])
          wm[im] = w[i];

      Vm = Vp;
    }

  glm::vec2 v = glm::normalize (P[j0] - P[i0]);
  glm::vec2 u = P[k0] - P[i0];
  u = glm::normalize (u - glm::dot (u, v) * v);

  // Pointing outside of triangle
  if (glm::dot (u, Vp) < 0.0f)
    {
      std::swap (Vp, Vm); std::swap (wp, wm); std::swap (itp, itm);
    }

}

void FieldStream::computeStreamLineDir 
  (int it, const BufferPtr<float> & ru, const BufferPtr<float> & rv, 
   const glm::vec2 & M0, const glm::vec2 & V0,
   stream_seen_t & seen, float sign, std::valarray<float> w,
   std::vector<glm::vec3> & list, const const_GeometryPtr & geometry)
{
  glm::vec2 V = V0, M = M0;
  bool again_flag = false;

  while (1)
    {

      if (seen[it])
        break;

      seen.add (it);

      int jglo[3], itri[3];

      glm::vec2 P[3];
      geometry->getTriangleNeighbours (it, jglo, itri, P);

      geometry->fixPeriodicity (M, P, 3);

      // Try all edges : intersection of vector line with triangle edges
      for (int i = 0; i < 3; i++)
        {
          int j = (i + 1) % 3;
          int k = (i + 2) % 3;

          if ((w[i] != 0.0f) && (w[j] != 0.0f)) // Exit triangle through other edge
            continue;

          float lambda = lineLineIntersect (M, sign * V, P[i], P[j]);

          if ((lambda < 0.0f) || (1.0f < lambda)) // Cross edge line between P[i] and P[j]
            continue;

          bool lpoint = (lambda == 0.0f) || (lambda == 1.0f);
           
          int itn = itri[i], jglon[3];

          if (itn < 0)
            {
              if (lpoint)
                continue;
              else  
                goto last;
            }

          if (seen[itn])
            {
              if (lpoint)
                continue;
              else
                goto last;
            }
          
          M = P[i] * lambda + P[j] * (1.0f - lambda);

          V = glm::vec2 (lambda * ru[jglo[i]] + (1.0f - lambda) * ru[jglo[j]],
                         lambda * rv[jglo[i]] + (1.0f - lambda) * rv[jglo[j]]);
 
          list.push_back (glm::vec3 (M.x, M.y, glm::length (V)));

          glm::vec2 v = glm::normalize (P[j] - P[i]);
	  glm::vec2 u = P[k] - P[i];
	  u = u - glm::dot (u, v) * v;

          if ((! again_flag) && (glm::dot (u, sign * V) > 0.0f))
            {
              // Special case
              seen.del (it);
              w[i] = lambda;
	      w[j] = 1.0f - lambda;
	      w[k] = 0.0f;
	      again_flag = true;
	    }
	  else
            {
              glm::vec3 xyzn[3]; int itrin[3];
              geometry->getTriangleNeighbours (itn, jglon, itrin, xyzn);

              w[0] = w[1] = w[2] = 0.0f;

              // Find weights for next triangle
              for (int k = 0; k < 3; k++)
                {
                  if (jglon[k] == jglo[i])
                    w[k] = lambda;
                  if (jglon[k] == jglo[j])
                    w[k] = 1.0f - lambda;
                }

              it = itn;

	      again_flag = false;
	    }

          break;
        }

    }

last:

  return;
}


void FieldStream::computeStreamLine 
  (int it0, const BufferPtr<float> & ru, const BufferPtr<float> & rv, 
   streamline_data_t * stream, const const_GeometryPtr & geometry)
{
  std::vector<glm::vec3> listf, listb;

  stream_seen_t seen;
  
  int itp, itm;
  std::valarray<float> wp (3), wm (3);
  glm::vec2 Vp, Vm, M;

  getFirstPoint (it0, ru, rv, M, Vp, Vm, wp, wm, itp, itm, geometry);

  // Forward 
  listf.push_back (glm::vec3 (M.x, M.y, glm::length (Vp)));

  if (itp >= 0)
    computeStreamLineDir (itp, ru, rv, M, Vp, seen, +1.0f, wp, listf, geometry);

  // Backward
  
  if (itm >= 0)
    computeStreamLineDir (itm, ru, rv, M, Vm, seen, -1.0f, wm, listb, geometry);

  // Add points to stream
  for (int i = listb.size () - 1; i >= 0; i--)
    stream->push (geometry->conformal2xyz (glm::vec2 (listb[i].x, listb[i].y)), listb[i].z);
  for (size_t i = 0; i < listf.size (); i++)
    stream->push (geometry->conformal2xyz (glm::vec2 (listf[i].x, listf[i].y)), listf[i].z);

  if (listb.size () + listf.size () > 0)
    stream->push (0.0f, 0.0f, 0.0f, 0.0f);

  return;
}

void FieldStream::streamline_t::render 
  (const bool & wide, const float & Width, const View & view) const
{
  Program * program = Program::load ("STREAM");
  if (wide)
    {
      float width = view.pixelToDistAtNadir (Width);
      program->set ("width", width);
      unsigned int ind[12] = {1, 0, 2, 3, 1, 2, 1, 3, 4, 1, 4, 5};
      glDrawElementsInstanced (GL_TRIANGLES, 12, GL_UNSIGNED_INT, ind, d.size);
    }
  else
    {
      glDrawArraysInstanced (GL_LINE_STRIP, 0, 2, d.size);
    }
}

void FieldStream::render (const View & view, const OptionsLight & light) const
{
  Program * program = Program::load ("STREAM");
  program->use ();

  view.setMVP (program);
  program->set ("scale0", opts.scale);

  palette.set (program);

  std::vector<float> valmin = getMinValue ();
  std::vector<float> valmax = getMaxValue ();

  program->set ("valmin", valmin[0]);
  program->set ("valmax", valmax[0]);
  program->set ("normmax", valmax[0]);
  program->set ("accelt", opts.stream.motion.timeaccel);
  program->set ("nwaves", opts.stream.motion.nwaves);

  double timea = getTime () - d.time0;
  program->set ("timea", float (timea));

  bool wide = opts.stream.width > 0.0f;
  float Width = 5.0f * opts.stream.width;

  program->set ("motion", opts.stream.motion.on);
  program->set ("scalenorm", ! opts.stream.motion.on);

  for (const auto & s : d.stream)
    s.VAID.render (wide, Width, view);

  view.delMVP (program);

}

}
