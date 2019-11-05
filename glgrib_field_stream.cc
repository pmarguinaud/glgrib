#include "glgrib_field_stream.h"
#include "glgrib_program.h"
#include "glgrib_palette.h"

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <algorithm>

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
    {
      for (int i = 0; i < stream.size (); i++)
        glDeleteVertexArrays (1, &stream[i].VertexArrayID);
    }
  glgrib_field::clear ();
}

void glgrib_field_stream::setupVertexAttributes ()
{
  numberOfPoints = geometry->numberOfPoints;
  numberOfTriangles = geometry->numberOfTriangles;

  for (int i = 0; i < stream.size (); i++)
    {
      glGenVertexArrays (1, &stream[i].VertexArrayID);
      glBindVertexArray (stream[i].VertexArrayID);
     
      stream[i].vertexbuffer->bind (GL_ARRAY_BUFFER);
     
      for (int j = 0; j < 3; j++)
        {
          glEnableVertexAttribArray (j);
          glVertexAttribPointer (j, 3, GL_FLOAT, GL_FALSE, 0, (const void *)(j * 3 * sizeof (float)));
          glVertexAttribDivisor (j, 1);
        }
     
      stream[i].normalbuffer->bind (GL_ARRAY_BUFFER);
     
      for (int j = 0; j < 2; j++)
        {
          glEnableVertexAttribArray (3 + j);
          glVertexAttribPointer (3 + j, 1, GL_FLOAT, GL_FALSE, 0, (const void *)(j * sizeof (float)));
          glVertexAttribDivisor (3 + j, 1);
        }
     
      stream[i].distancebuffer->bind (GL_ARRAY_BUFFER);
     
      for (int j = 0; j < 2; j++)
        {
          glEnableVertexAttribArray (5 + j); 
          glVertexAttribPointer (5 + j, 1, GL_FLOAT, GL_FALSE, 0, (const void *)(j * sizeof (float))); 
          glVertexAttribDivisor (5 + j, 1);
        }
     
      glBindVertexArray (0); 
    }
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
//    (*data_u)[i] = 1.0f;
//    (*data_v)[i] = 1.0f;
    }

  bool * seen = (bool *)malloc (sizeof (bool) * geometry->numberOfTriangles);
  for (int i = 0; i < geometry->numberOfTriangles; i++)
    seen[i] = false;

  std::vector<streamline_data_t> stream_data;

  int N = 16;

  stream_data.resize (N);
  stream.resize (N);

  std::vector<int> it;

  unsigned char * sample = new unsigned char[geometry->numberOfTriangles];
  for (int i = 0; i < geometry->numberOfTriangles; i++)
    sample[i] = 0;

  geometry->sampleTriangle (sample, 1, 50);

  for (int i = 0; i < geometry->numberOfTriangles; i++)
    if (sample[i])
      it.push_back (i);
  delete [] sample;

#pragma omp parallel for
  for (int j = 0; j < N; j++)
    {
      int i0 = ((j + 0) * it.size ()) / N;
      int i1 = ((j + 1) * it.size ()) / N;
      for (int i = i0; i < i1; i++)
        processTriangle (it[i], data_u->data (), data_v->data (), 
                         seen, &stream_data[j]);
    }

  free (seen);


  for (int i = 0; i < N; i++)
    {
      stream[i].vertexbuffer   = new_glgrib_opengl_buffer_ptr (stream_data[i].xyz.size () * sizeof (float), stream_data[i].xyz.data ());
      stream[i].normalbuffer   = new_glgrib_opengl_buffer_ptr (stream_data[i].drw.size () * sizeof (float), stream_data[i].drw.data ());
      stream[i].distancebuffer = new_glgrib_opengl_buffer_ptr (stream_data[i].dis.size () * sizeof (float), stream_data[i].dis.data ());
      stream[i].size = stream_data[i].size () - 1;
    }

  setupVertexAttributes ();

  setReady ();
}

float lineLineIntersect (const glm::vec2 & P1, const glm::vec2 & V1,
                         const glm::vec2 & P,  const glm::vec2 & Q)
{
  const float inf = std::numeric_limits<float>::infinity ();

  float b = V1.x * (P.y - Q.y ) - V1.y * (P.x - Q.x ); 
  float a = V1.y * (Q.x - P1.x) - V1.x * (Q.y - P1.y);

  if (fabsf (b) < 1e-6)
    return inf;

  float lambda = a / b;

  glm::vec2 M = P * lambda + Q * (1.0f - lambda);

  if (glm::dot (M - P1, V1) < 0.0f)
    return inf;

  return lambda;
}


void glgrib_field_stream::getFirstPoint (int it, const float * ru, const float * rv, 
					 glm::vec2 & M, glm::vec2 & Vp, glm::vec2 & Vm,
		                         std::valarray<float> & wp, std::valarray<float> & wm, 
					 int & itp, int & itm)
{
  // First point of the list; see which segment to start with, initialize weights, V and M
  int jglo[3], itri[3];

  std::valarray<float> w (3);
  glm::vec2 P[3];

  geometry->getTriangleNeighbours (it, jglo, itri, P);

  
  int i0;

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
      if (fabsf (s) < fabsf (s0))
        {
          s0 = s;
          i0 = i;
        }
    }
  
  int j0 = (i0 + 1) % 3;
  int k0 = (i0 + 2) % 3;
  
  w[i0] = w[j0] = 0.5f; w[k0] = 0.0f;
  
  M = (w[0] * P[0] + w[1] * P[1] + w[2] * P[2]) / w.sum ();

  glm::vec2 V = glm::vec2 (w[0] * ru[jglo[0]] + w[1] * ru[jglo[1]] + w[2] * ru[jglo[2]],
                           w[0] * rv[jglo[0]] + w[1] * rv[jglo[1]] + w[2] * rv[jglo[2]]) 
	                 / w.sum ();

  Vp = V; itp = it; wp = w;

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

  glm::vec2 v = glm::normalize (P[k0] - P[i0]);
  glm::vec2 u = P[k0] - P[i0];
  u = glm::normalize (u - glm::dot (u, v));

  // Pointing outside of triangle
  if (glm::dot (u, Vp) < 0.0f)
    {
      std::swap (Vp, Vm); std::swap (wp, wm); std::swap (itp, itm);
//    std::cout << " Vp = " << to_string (Vp) << std::endl;
//    std::cout << " Vm = " << to_string (Vm) << std::endl;
//    std::cout << " itp = " << itp << std::endl;
//    std::cout << " itm = " << itm << std::endl;
    }

}

void glgrib_field_stream::processTriangle1 (int it, const float * ru, const float * rv, 
		                            const glm::vec2 & M0, const glm::vec2 & V0,
                                            bool * seen, float sign, std::valarray<float> w,
					    std::vector<glm::vec3> & list)
{
  glm::vec2 V = V0, M = M0;
  bool again_flag = false;

  while (1)
    {

      if (seen[it])
        break;

      seen[it] = true;

      int jglo[3], itri[3];

      glm::vec2 P[3];
      geometry->getTriangleNeighbours (it, jglo, itri, P);

      // Fix periodicity issue
      for (int i = 0; i < 3; i++)
        {
          if (M.x - P[i].x > M_PI)
            P[i].x += 2.0f * M_PI;
          else 
          if (P[i].x - M.x > M_PI)
            P[i].x -= 2.0f * M_PI;
        }

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
              seen[it] = false;
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


void glgrib_field_stream::processTriangle (int it0, const float * ru, const float * rv, 
                                           bool * seen, streamline_data_t * stream)
{
  if (seen[it0])
    return;

  std::vector<glm::vec3> listf, listb;

  bool * seen_loc = (bool *)malloc (sizeof (bool) * geometry->numberOfTriangles);
  for (int i = 0; i < geometry->numberOfTriangles; i++)
    seen_loc[i] = false;
  
  int itp, itm;
  std::valarray<float> wp (3), wm (3);
  glm::vec2 Vp, Vm, M;
  getFirstPoint (it0, ru, rv, M, Vp, Vm, wp, wm, itp, itm);

  // Forward 
  listf.push_back (glm::vec3 (M.x, M.y, glm::length (Vp)));

  if (itp >= 0)
    processTriangle1 (itp, ru, rv, M, Vp, seen_loc, +1.0f, wp, listf);

  // Backward
  
  if (itm >= 0)
    processTriangle1 (itm, ru, rv, M, Vm, seen_loc, -1.0f, wm, listb);

  // Add points to stream
  for (int i = listb.size () - 1; i >= 0; i--)
    stream->push (geometry->conformal2xyz (glm::vec2 (listb[i].x, listb[i].y)), listb[i].z);
  for (int i = 0; i < listf.size (); i++)
    stream->push (geometry->conformal2xyz (glm::vec2 (listf[i].x, listf[i].y)), listf[i].z);

  if (listb.size () + listf.size () > 0)
    stream->push (0.0f, 0.0f, 0.0f, 0.0f);

  free (seen_loc);

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



  float color0[3] = {  0.0f/255.0f, 
                       0.0f/255.0f, 
                     255.0f/255.0f};
  program->set3fv ("color0", color0);

  bool wide = true;
  float Width = 5.0f;

  for (int i = 0; i < stream.size (); i++)
    {
      glBindVertexArray (stream[i].VertexArrayID);
      if (wide)
        {
          float width = view.pixel_to_dist_at_nadir (Width);
          program->set1f ("width", width);
          unsigned int ind[12] = {1, 0, 2, 3, 1, 2, 1, 3, 4, 1, 4, 5};
          glDrawElementsInstanced (GL_TRIANGLES, 12, GL_UNSIGNED_INT, ind, stream[i].size);
        }
      else
        {
          glDrawArraysInstanced (GL_LINE_STRIP, 0, 2, stream[i].size);
        }
    }

  glBindVertexArray (0);

}

glgrib_field_stream::~glgrib_field_stream ()
{
}


