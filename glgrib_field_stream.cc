#include "glgrib_field_stream.h"
#include "glgrib_program.h"
#include "glgrib_palette.h"

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <algorithm>
#include <bits/stdc++.h> 

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>


glgrib_field_stream::glgrib_field_stream (const glgrib_field_stream & field)
{
  if (field.isReady ())
    {
      // Cleanup already existing VAOs
      clear ();
      operator= (field);
    }
}

glgrib_field_stream * glgrib_field_stream::clone () const
{
  if (this == NULL)
    return NULL;
  glgrib_field_stream * fld = new glgrib_field_stream ();
  *fld = *this;
  return fld;
}

glgrib_field_stream & glgrib_field_stream::operator= (const glgrib_field_stream & field)
{
  if (this != &field)
    {
      clear ();
      if (field.isReady ())
        {
          glgrib_field::operator= (field);
          stream = field.stream;
          setupVertexAttributes ();
          setReady ();
        }
    }
}

void glgrib_field_stream::clear ()
{
  if (isReady ()) 
    glDeleteVertexArrays (1, &stream.VertexArrayID);
  glgrib_field::clear ();
}

void glgrib_field_stream::setupVertexAttributes ()
{
  numberOfPoints = geometry->numberOfPoints;
  numberOfTriangles = geometry->numberOfTriangles;

  glGenVertexArrays (1, &stream.VertexArrayID);
  glBindVertexArray (stream.VertexArrayID);

  stream.vertexbuffer->bind (GL_ARRAY_BUFFER);

  for (int j = 0; j < 3; j++)
    {
      glEnableVertexAttribArray (j);
      glVertexAttribPointer (j, 3, GL_FLOAT, GL_FALSE, 0, (const void *)(j * 3 * sizeof (float)));
      glVertexAttribDivisor (j, 1);
    }

  stream.normalbuffer->bind (GL_ARRAY_BUFFER);

  for (int j = 0; j < 2; j++)
    {
      glEnableVertexAttribArray (3 + j);
      glVertexAttribPointer (3 + j, 1, GL_FLOAT, GL_FALSE, 0, (const void *)(j * sizeof (float)));
      glVertexAttribDivisor (3 + j, 1);
    }

  stream.distancebuffer->bind (GL_ARRAY_BUFFER);

  for (int j = 0; j < 2; j++)
    {
      glEnableVertexAttribArray (5 + j); 
      glVertexAttribPointer (5 + j, 1, GL_FLOAT, GL_FALSE, 0, (const void *)(j * sizeof (float))); 
      glVertexAttribDivisor (5 + j, 1);
    }

  glBindVertexArray (0); 
}

void glgrib_field_stream::setup (glgrib_loader * ld, const glgrib_options_field & o, float slot)
{
  opts = o;

  glgrib_field_metadata meta_u, meta_v;
  glgrib_field_metadata meta_n, meta_d;

  glgrib_field_float_buffer_ptr data_u = ld->load (opts.path, slot, &meta_u, 2, 0);
  glgrib_field_float_buffer_ptr data_v = ld->load (opts.path, slot, &meta_v, 2, 1);

  geometry = glgrib_geometry_load (ld, opts.path[0]);

  numberOfColors = 1;

  int size = geometry->size ();

  float normmax = 0.0f;
  for (int i = 0; i < size; i++)
    normmax = std::max (normmax, (*data_u)[i] * (*data_u)[i] + (*data_v)[i] * (*data_v)[i]);

  normmax = sqrt (normmax);

  for (int i = 0; i < size; i++)
    {
      (*data_u)[i] = (*data_u)[i] / normmax;
      (*data_v)[i] = (*data_v)[i] / normmax;
    }

  bool * seen = (bool *)malloc (sizeof (bool) * geometry->numberOfTriangles);
  for (int i = 0; i < geometry->numberOfTriangles; i++)
    seen[i] = false;


  streamline_data_t stream_data;

  int dit = 1;

  std::cout << " geometry->numberOfTriangles  = " << geometry->numberOfTriangles << std::endl;

  for (int it = 0; it < geometry->numberOfTriangles; it += dit)
    processTriangle (it, data_u->data (), data_v->data (), 
                     seen, &stream_data);

  free (seen);


  stream.vertexbuffer   = new_glgrib_opengl_buffer_ptr (stream_data.xyz.size () * sizeof (float), 
                                                        stream_data.xyz.data ());
  stream.normalbuffer   = new_glgrib_opengl_buffer_ptr (stream_data.drw.size () * sizeof (float), 
                                                        stream_data.drw.data ());
  stream.distancebuffer = new_glgrib_opengl_buffer_ptr (stream_data.dis.size () * sizeof (float), 
                                                        stream_data.dis.data ());
  stream.size = stream_data.size () - 1;

  setupVertexAttributes ();

  setReady ();
}

static glm::vec2 xyz2merc (const glm::vec3 & xyz)
{
  float lon = atan2 (xyz.y, xyz.x);
  float lat = asin (xyz.z);
  return glm::vec2 (lon, log (tan (M_PI / 4.0f + lat / 2.0f)));
}

static glm::vec3 merc2xyz (const glm::vec2 & merc)
{
  float lon = merc.x;
  float lat = 2.0f * atan (exp (merc.y)) - M_PI / 2.0f;
  float coslon = cos (lon), sinlon = sin (lon);
  float coslat = cos (lat), sinlat = sin (lat);

  return glm::vec3 (coslon * coslat, sinlon * coslat, sinlat);
}

static glm::vec2 merc2lonlat (const glm::vec2 & merc)
{
  float lon = merc.x;
  float lat = 2.0f * atan (exp (merc.y)) - M_PI / 2.0f;
  return glm::vec2 (glm::degrees (lon), glm::degrees (lat));
}

float lineLineIntersect (const glm::vec2 & P1, const glm::vec2 & V1,
                         const glm::vec2 & P,  const glm::vec2 & Q)
{
  float b = V1.x * (P.y - Q.y ) - V1.y * (P.x - Q.x ); 
  float a = V1.y * (Q.x - P1.x) - V1.x * (Q.y - P1.y);

  if (fabsf (b) < 1e-6)
    return std::numeric_limits<float>::infinity ();

  return a / b;
}

void glgrib_field_stream::processTriangle (int it0, float * ru, float * rv, 
                                           bool * seen, streamline_data_t * stream)
{
  std::vector<glm::vec3> listf, listb, * list = &listf;

  int it = it0; 
  std::valarray<float> w0 (3), w (3);
  glm::vec2 M, V;   // Current values
  glm::vec2 M0, V0; // Previous values
  float V0MM00;

  bool dbg = false;

  int count = 0;
  
  while (1)
    {

dbg = (it0 == 92186);
if (dbg)
std::cout << " it = " << it << " " << seen[it] << std::endl;


      if (seen[it])
        goto last;


if (dbg) dbg = (list == &listf) && (it == 92189);

if (dbg) std::cout << __FILE__ << ":" << __LINE__ << std::endl;

      seen[it] = true;

      //
      {
        int jglo[3], itri[3];
        glm::vec3 xyz[3];

        geometry->getTriangleNeighbours (it, jglo, itri, xyz);

if (dbg) std::cout << __FILE__ << ":" << __LINE__ << " jglo = " << jglo[0] << " " << jglo[1] << " " << jglo[2] << std::endl;
if (dbg) std::cout << __FILE__ << ":" << __LINE__ << " itri = " << itri[0] << " " << itri[1] << " " << itri[2] << std::endl;

        glm::vec2 Mn;
        glm::vec2 P[3];
   
        // Mercator projection
        for (int i = 0; i < 3; i++)
          P[i] = xyz2merc (xyz[i]);
   
        // First point of the list; see which segment to start with, initialize weights, V and M
        if (listf.size () == 0) 
          {
            int i0;

            float s0 = std::numeric_limits<float>::max ();
            for (int i = 0; i < 3; i++)
              {
                int j = (i + 1) % 3;
                int k = (i + 2) % 3;
                w[i] = w[j] = 0.5f; w[k] = 0.0f;
                V = glm::vec2 (w[0] * ru[jglo[0]] + w[1] * ru[jglo[1]] + w[2] * ru[jglo[2]],
                               w[0] * rv[jglo[0]] + w[1] * rv[jglo[1]] + w[2] * rv[jglo[2]]) / w.sum ();
                V = glm::normalize (V);
                glm::vec2 U = glm::normalize (P[j] - P[i]);
                float s = fabsf (glm::dot (U, V));
                if (s < s0)
                  {
                    s0 = s;
                    i0 = i;
                  }
              }
            
            int j0 = (i0 + 1) % 3;
            int k0 = (i0 + 2) % 3;
     
            w[i0] = w[j0] = 0.5f; w[k0] = 0.0f;
   
            M = (w[0] * P[0] + w[1] * P[1] + w[2] * P[2]) / w.sum ();
   
            w0 = w;

            V = glm::vec2 (w[0] * ru[jglo[0]] + w[1] * ru[jglo[1]] + w[2] * ru[jglo[2]],
                           w[0] * rv[jglo[0]] + w[1] * rv[jglo[1]] + w[2] * rv[jglo[2]]) / w.sum ();
   
            list->push_back (glm::vec3 (M.x, M.y, glm::length (V)));

          }

if (dbg) std::cout << __FILE__ << ":" << __LINE__ << std::endl;


        if (list->size () >= 2)
          {

            if (M.x - M0.x > M_PI)
              M0.x += 2.0f * M_PI;
            else if (M0.x - M.x > M_PI)
              M0.x -= 2.0f * M_PI;

            float V0MM0 = glm::dot (V0, M-M0);

            if ((list == &listf) && (list->size () == 2))
              {
                V0MM00 = V0MM0;
              }
            else if (list == &listf)
              {
                if (V0MM00 * V0MM0 < 0.0f)
                  goto last;
              }
            else if (list == &listb)
              {
                if (V0MM00 * V0MM0 > 0.0f)
                  goto last;
              }
          }
   
if (dbg) std::cout << __FILE__ << ":" << __LINE__ << std::endl;

        // Fix periodicity issue
        for (int i = 0; i < 3; i++)
          {
            if (M.x - P[i].x > M_PI)
              P[i].x += 2.0f * M_PI;
            else if (P[i].x - M.x > M_PI)
              P[i].x -= 2.0f * M_PI;
          }

        M0 = M; V0 = V;

if (dbg)
{
  std::cout << " M = " << glm::to_string (merc2lonlat (glm::vec2 (M.x, M.y))) << std::endl;
  std::cout << " V = " << glm::to_string (merc2lonlat (glm::vec2 (V.x, V.y))) << std::endl;
}


if (dbg) std::cout << __FILE__ << ":" << __LINE__ << " w " << w[0] << " " << w[1] << " " << w[2] << std::endl;

        // Try all edges : intersection of vector line with triangle edges
        for (int i = 0; i < 3; i++)
          {
            int j = (i + 1) % 3;
            int k = (i + 2) % 3;

            if ((w[i] != 0.0f) && (w[j] != 0.0f)) // Exit triangle through other edge
              continue;

if (dbg) std::cout << __FILE__ << ":" << __LINE__ << " " << i << " " << j << " " << k << std::endl;
if (dbg) std::cout << __FILE__ << ":" << __LINE__ << " w " << w[0] << " " << w[1] << " " << w[2] << std::endl;
if (dbg) std::cout << __FILE__ << ":" << __LINE__ << " w[i], w[j] = " << w[i] << " " << w[j] << std::endl;

            float lambda = lineLineIntersect (M, V, P[i], P[j]);

if (dbg) std::cout << __FILE__ << ":" << __LINE__ << " lambda = " << lambda << std::endl;

            if ((lambda < 0.0f) || (1.0f < lambda)) // Cross edge line between P[i] and P[j]
              continue;

            M = P[i] * lambda + P[j] * (1.0f - lambda);

            V = glm::vec2 (lambda * ru[jglo[i]] + (1.0f - lambda) * ru[jglo[j]],
                           lambda * rv[jglo[i]] + (1.0f - lambda) * rv[jglo[j]]);

 
            list->push_back (glm::vec3 (M.x, M.y, glm::length (V)));

            int itn = itri[i], jglon[3];

if (dbg) std::cout << __FILE__ << ":" << __LINE__ << " itn = " << itn << std::endl;


            if (itn < 0)
              {
//              list->pop_back ();
                continue;
              }
if (dbg) std::cout << __FILE__ << ":" << __LINE__ << " seen = " << seen[itn] << std::endl;

            if (seen[itn])
              {
                list->pop_back ();
                continue;
              }
            
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

            goto found;
          }

       }

      goto last;

found:
  
      count++;

      continue;

last:

      if ((list == &listf) && (listf.size () > 0))
        {
          M = listf[0]; w = w0;
          list = &listb;
          int jglo[3], itri[3];
          glm::vec3 xyz[3];

          geometry->getTriangleNeighbours (it0, jglo, itri, xyz);

          for (int i = 0; i < 3; i++)
            {
              int j = (i + 1) % 3;
              if ((w[i] > 0.0f) && (w[j] > 0.0f))
                {
                  it = itri[i];
                  break;
                }
            }

          if (it == -1)
            break;

          continue;
        }

      break;

    }

if (dbg)
  {
    std::cout << " it0 = " << it0 << std::endl;
    std::cout << " listb.size () = " << listb.size () << std::endl;

    for (int i = 0; i < listb.size (); i++)
      {
        glm::vec2 latlon = merc2lonlat (glm::vec2 (listb[i].x, listb[i].y));
        printf ("%12.5f %12.5f\n", latlon.x, latlon.y);
      }

    std::cout << " listf.size () = " << listf.size () << std::endl;

    for (int i = 0; i < listf.size (); i++)
      {
        glm::vec2 latlon = merc2lonlat (glm::vec2 (listf[i].x, listf[i].y));
        printf ("%12.5f %12.5f\n", latlon.x, latlon.y);
      }
  }

  if (V0MM00 > 0.0f)
    {
      for (int i = listb.size () - 1; i >= 0; i--)
        stream->push (merc2xyz (glm::vec2 (listb[i].x, listb[i].y)), listb[i].z);
      for (int i = 0; i < listf.size (); i++)
        stream->push (merc2xyz (glm::vec2 (listf[i].x, listf[i].y)), listf[i].z);
    }
  else
    {
      for (int i = listf.size () - 1; i >= 0; i--)
        stream->push (merc2xyz (glm::vec2 (listf[i].x, listf[i].y)), listf[i].z);
      for (int i = 0; i < listb.size (); i++)
        stream->push (merc2xyz (glm::vec2 (listb[i].x, listb[i].y)), listb[i].z);
    }

  if (false && (stream->dis[stream->size ()-1] < 0.1000f)) // About 5 degrees
    {
  std::cout << " POP ! " << std::endl;
      for (int i = 0; i < listf.size () + listb.size (); i++)
        stream->pop ();
    }
  else
    {
      if (listf.size () + listb.size () > 0)
        stream->push (0.0f, 0.0f, 0.0f, 0.0f);
    }





  return;
}

void glgrib_field_stream::render (const glgrib_view & view, const glgrib_options_light & light) const
{
  glgrib_program * program = glgrib_program_load (glgrib_program::STREAM);
  program->use ();
  float scale0[3] = {opts.scale, opts.scale, opts.scale};
  const glgrib_palette & p = palette;

  view.setMVP (program);
  program->set3fv ("scale0", scale0);


  glBindVertexArray (stream.VertexArrayID);

  float color0[3] = {255.0f/255.0f, 
                     255.0f/255.0f, 
                     255.0f/255.0f};
  program->set3fv ("color0", color0);

  bool wide = true;
  float Width = 5.0f;
  if (wide)
    {
      float width = view.pixel_to_dist_at_nadir (Width);
      program->set1f ("width", width);
      unsigned int ind[12] = {1, 0, 2, 3, 1, 2, 1, 3, 4, 1, 4, 5};
      glDrawElementsInstanced (GL_TRIANGLES, 12, GL_UNSIGNED_INT, ind, stream.size);
    }
  else
    {
      glDrawArraysInstanced (GL_LINE_STRIP, 0, 2, stream.size);
    }

  glBindVertexArray (0);

}

glgrib_field_stream::~glgrib_field_stream ()
{
}


