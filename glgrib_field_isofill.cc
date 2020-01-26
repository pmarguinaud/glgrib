#include "glgrib_field_isofill.h"
#include "glgrib_trigonometry.h"
#include "glgrib_program.h"
#include "glgrib_palette.h"

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <algorithm>

#include <glm/glm.hpp>
#include <omp.h>


glgrib_field_isofill::glgrib_field_isofill (const glgrib_field_isofill & field)
{
  if (field.isReady ())
    {
      // Cleanup already existing VAOs
      clear ();
      operator= (field);
    }
}

glgrib_field_isofill * glgrib_field_isofill::clone () const
{
  if (this == NULL)
    return NULL;
  glgrib_field_isofill * fld = new glgrib_field_isofill ();
  *fld = *this;
  return fld;
}

glgrib_field_isofill & glgrib_field_isofill::operator= (const glgrib_field_isofill & field)
{
  if (this != &field)
    {
      clear ();
      if (field.isReady ())
        {
          glgrib_field::operator= (field);
          d = field.d;
          setupVertexAttributes ();
          setReady ();
        }
    }
  return *this;
}

void glgrib_field_isofill::clear ()
{
  if (isReady ()) 
    {
      glDeleteVertexArrays (1, &d.VertexArrayID);
      for (int i = 0; i < d.isoband.size (); i++)
        {
          glDeleteVertexArrays (1, &d.isoband[i].VertexArrayID);
        }
    }
  glgrib_field::clear ();
}

void glgrib_field_isofill::setupVertexAttributes ()
{
  numberOfPoints = geometry->getNumberOfPoints ();
  numberOfTriangles = geometry->getNumberOfTriangles ();

  // Triangles from original geometry
  glGenVertexArrays (1, &d.VertexArrayID);
  glBindVertexArray (d.VertexArrayID);

  // Elements
  geometry->bindTriangles ();

  // Coordinates
  geometry->bindCoordinates (0);
  glEnableVertexAttribArray (0);
  glVertexAttribPointer (0, 2, GL_FLOAT, GL_FALSE, 0, NULL);

  d.colorbuffer->bind (GL_ARRAY_BUFFER);
  glEnableVertexAttribArray (1);

  glVertexAttribPointer (1, 1, GL_UNSIGNED_BYTE, GL_FALSE, 0,  NULL);

  glBindVertexArray (0); 

  for (int i = 0; i < d.isoband.size (); i++)
    {
      // New triangles
      glGenVertexArrays (1, &d.isoband[i].VertexArrayID);
      glBindVertexArray (d.isoband[i].VertexArrayID);
  
      // Elements
      d.isoband[i].elementbuffer->bind (GL_ELEMENT_ARRAY_BUFFER);
      d.isoband[i].vertexbuffer->bind (GL_ARRAY_BUFFER);
  
      glEnableVertexAttribArray (0);
      glVertexAttribPointer (0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
  
      glBindVertexArray (0); 
    }
}

class isoband_maker_t
{
public:

  int offset_indice, length_indice;
  int offset_lonlat, length_lonlat;

  std::vector<unsigned int> indice;
  std::vector<float> lonlat;

  void push_lonlat (const glm::vec2 & lonlat_)
  {
    lonlat.push_back (lonlat_.x);
    lonlat.push_back (lonlat_.y);
  }

  template <typename T, typename... Args>
  void push_lonlat (const T & lonlat, Args... args)
  {
    push_lonlat (lonlat);
    push_lonlat (args...);
  }

  void push_indice (int ind)
  {
    indice.push_back (ind);
  }

  template <typename T, typename... Args>
  void push_indice (T ind, Args... args)
  {
    push_indice (ind);
    push_indice (args...);
  }

  void quad (const glm::vec2 & lonlata, const glm::vec2 & lonlatb, 
             const glm::vec2 & lonlatc, const glm::vec2 & lonlatd,
             bool direct)
  {
    int ind0 = lonlat.size () / 2;

    push_lonlat (lonlata, lonlatb, 
                 lonlatc, lonlatd);
    
    int ord[6] = {0, 1, 2, 0, 2, 3};

    if (! direct)
      {
        std::swap (ord[0], ord[1]);
        std::swap (ord[3], ord[4]);
      }

    push_indice (ind0+ord[0], ind0+ord[1], 
                 ind0+ord[2], ind0+ord[3], 
                 ind0+ord[4], ind0+ord[5]);

  }

  void tri (const glm::vec2 & lonlata, 
            const glm::vec2 & lonlatb, 
            const glm::vec2 & lonlatc)
  {
    int ind0 = lonlat.size () / 2;

    push_lonlat (lonlata, lonlatb, lonlatc);
    push_indice (ind0+0, ind0+1, ind0+2);
  }

  void penta (const glm::vec2 & lonlata, const glm::vec2 & lonlatb, 
        	const glm::vec2 & lonlatc, const glm::vec2 & lonlatd,
        	const glm::vec2 & lonlate)
  {
    int ind0 = lonlat.size () / 2;

    push_lonlat (lonlata, lonlatb, lonlatc,
                 lonlatd, lonlate);

    push_indice (ind0+0, ind0+1, ind0+2,
                 ind0+0, ind0+2, ind0+3,
                 ind0+0, ind0+3, ind0+4);
  }
 
};


static
void processTriangle2 (std::vector<isoband_maker_t> * isomake, 
                       const float v[3], const glm::vec3 xyz[3],
                       const std::vector<float> & levels)
{

  class processTriangle2_ctx_t
  {
  public:
    int I = -1;
    glm::vec2 lonlat_J, lonlat_K;
    glm::vec2 lonlat[3];
    const float * v = NULL;
  };

  processTriangle2_ctx_t ctx;

  for (int i = 0; i < 3; i++)
    ctx.lonlat[i] = xyz2lonlat (xyz[i]);
  ctx.v = v;


  auto close = [&ctx] (isoband_maker_t & ib)
  {
    int i = std::max_element (ctx.v, ctx.v + 3) - ctx.v;
    int j = (i + 1) % 3;
    int k = (j + 1) % 3;
    if (ctx.I == -1)
      {
        ib.tri (ctx.lonlat[i], ctx.lonlat[j], ctx.lonlat[k]);
      }
    else if (ctx.I == i)
      {
        ib.tri (ctx.lonlat[i], ctx.lonlat_J, ctx.lonlat_K);
      }
    else if (ctx.I == j)
      {
        ib.quad (ctx.lonlat[i], ctx.lonlat_K, ctx.lonlat_J, ctx.lonlat[k], true);
      }
    else if (ctx.I == k)
      {
        ib.quad (ctx.lonlat_K, ctx.lonlat_J, ctx.lonlat[i], ctx.lonlat[j], true);
      }
  };

  for (int ll = 0; ll < levels.size (); ll++)
    {
      float lev = levels[ll];

      isoband_maker_t & ib = (*isomake)[ll];

      bool b[3];
      for (int i = 0; i < 3; i++)
        b[i] = lev <= v[i];

      if ((! b[0]) && (! b[1]) && (! b[2]))
        {
          close (ib);
          return;
	}

      for (int i = 0; i < 3; i++)
        {
          int j = (i + 1) % 3;
          int k = (j + 1) % 3;
	  if ((b[i] != b[j]) && (b[i] != b[k]))
            {
              float a_j = (lev - v[i]) / (v[j] - v[i]);
              float a_k = (lev - v[i]) / (v[k] - v[i]);
	      glm::vec3 xyz_j = glm::normalize (xyz[j] * a_j + xyz[i] * (1.0f - a_j)),
	                xyz_k = glm::normalize (xyz[k] * a_k + xyz[i] * (1.0f - a_k));
	      glm::vec2 lonlat_j = xyz2lonlat (xyz_j),
	                lonlat_k = xyz2lonlat (xyz_k);
              
	      int ind0 = ib.lonlat.size () / 2;

	      if (ctx.I == -1)
                {
                  if (b[i])
                    ib.quad (lonlat_j, ctx.lonlat[j], ctx.lonlat[k], lonlat_k, b[i]);
                  else
                    ib.tri (ctx.lonlat[i], lonlat_j, lonlat_k);

		  ctx.I = i;
		  ctx.lonlat_J = lonlat_j;
		  ctx.lonlat_K = lonlat_k;
	        }
	      else
                {
                  if (i == ctx.I)
                    ib.quad (lonlat_j, ctx.lonlat_J, ctx.lonlat_K, lonlat_k, b[i]);
		  else if (j == ctx.I)
		    ib.penta (ctx.lonlat[k], lonlat_k, lonlat_j, ctx.lonlat_K, ctx.lonlat_J);
		  else if (k == ctx.I)
		    ib.penta (lonlat_j, ctx.lonlat[j], ctx.lonlat_K, ctx.lonlat_J, lonlat_k);

		  ctx.I = i;
		  ctx.lonlat_J = lonlat_j;
		  ctx.lonlat_K = lonlat_k;
		}

              break;
	    }
	}
    }


  close (isomake->back ());
}

static
void processTriangle1 (std::vector<isoband_maker_t> * isomake, 
                       const_glgrib_geometry_ptr geometry,
                       const float * val, int it, 
                       const std::vector<float> & levels)
{
  int jglo[3];
  
  geometry->getTriangleVertices (it, jglo);

  float v[3] = {val[jglo[0]], val[jglo[1]], val[jglo[2]]};

  if ((v[0] < levels.front ()) && (v[1] < levels.front ()) && (v[2] < levels.front ()))
    return;

  if ((v[0] > levels.back  ()) && (v[1] > levels.back  ()) && (v[2] > levels.back  ()))
    return;

  for (int i = 0; i < levels.size ()-1; i++)
    if (((v[0] > levels[i+0]) && (v[1] > levels[i+0]) && (v[2] > levels[i+0]))
     && ((v[0] < levels[i+1]) && (v[1] < levels[i+1]) && (v[2] < levels[i+1])))
      return;


  // At this point, we know the triangle will not have an homogeneous color;
  // we will have to split it into subtriangles

  glm::vec3 xyz[3];

  for (int i = 0; i < 3; i++)
    {
      float lon, lat;
      geometry->index2latlon (jglo[i], &lat, &lon);
      xyz[i] = lonlat2xyz (glm::vec2 (lon, lat));
    }


  processTriangle2 (isomake, v, xyz, levels);


}

void glgrib_field_isofill::setup (glgrib_loader * ld, const glgrib_options_field & o, float slot)
{
  opts = o;

  glgrib_field_metadata meta1;
  glgrib_field_float_buffer_ptr data;
  ld->load (&data, opts.path, opts.geometry, slot, &meta1, 1, 0, opts.diff.on);
  meta.push_back (meta1);

  palette = glgrib_palette::create (opts.palette, getNormedMinValue (), getNormedMaxValue ());

  geometry = glgrib_geometry::load (ld, opts.path[0], opts.geometry);

  int size = geometry->size ();

  std::vector<float> levels = opts.isofill.levels;

  std::cout << opts.isofill.min << ", " << opts.isofill.max << std::endl;

  if (levels.size () == 0)
    {
      float min = opts.isofill.min == glgrib_options_isofill::defaultMin 
                ? meta1.valmin : opts.isofill.min;
      float max = opts.isofill.max == glgrib_options_isofill::defaultMax 
                ? meta1.valmax : opts.isofill.max;
      for (int i = 0; i < opts.isofill.number; i++)
        levels.push_back (min + (i + 1) * (max - min) / (opts.isofill.number + 1));
    }

  d.isoband.resize (levels.size () + 1);
 

  int nth = omp_get_max_threads ();
  std::vector<isoband_maker_t> isomake[nth];

  for (int ith = 0; ith < nth; ith++)
    isomake[ith].resize (d.isoband.size ());


  std::vector<int> color_index (d.isoband.size ());

  for (int i = 0; i < d.isoband.size (); i++)
    {
      float v;
      if (i == 0)
        v = levels.front () * 0.99f - 1.0f;
      else if (i == d.isoband.size () - 1)
        v = levels.back  () * 1.01f + 1.0f;
      else
        v = (levels[i-1] + levels[i+0]) / 2.0f;

      d.isoband[i].color = palette.getColor      (v);
      color_index[i]     = palette.getColorIndex (v);
    }


  float * val = data->data ();

#pragma omp parallel for
  for (int it = 0; it < geometry->getNumberOfTriangles (); it++)
    {
      int ith = omp_get_thread_num ();
      processTriangle1 (&isomake[ith], geometry, val, it, levels);
    }


  {
    unsigned char * color = new unsigned char[size];

#pragma omp parallel for
    for (int i = 0; i < size; i++)
      {
        float v = val[i];
        if (v < levels.front ())
          color[i] = color_index.front ();
        else if (v > levels.back ())
          color[i] = color_index.back  ();
        else
          for (int j = 0; j < levels.size ()-1; j++)
            if ((levels[j] < v) && (v < levels[j+1]))
              {
                color[i] = color_index[j+1];
                break;
              }
      }

    d.colorbuffer  = new_glgrib_opengl_buffer_ptr (size * sizeof (unsigned char), color);
    
    delete [] color;
  }

  val = NULL;

  for (int i = 0; i < d.isoband.size (); i++)
    {

      // Compute offset/length of each thread for the current band
      int offset_lonlat = 0, 
          offset_indice = 0;

      for (int ith = 0; ith < nth; ith++)
        {
          isomake[ith][i].offset_indice = offset_indice;
          isomake[ith][i].offset_lonlat = offset_lonlat;
          isomake[ith][i].length_indice = isomake[ith][i].indice.size ();
          isomake[ith][i].length_lonlat = isomake[ith][i].lonlat.size ();
          offset_indice += isomake[ith][i].length_indice;
          offset_lonlat += isomake[ith][i].length_lonlat;
        }

      int length_lonlat = offset_lonlat, 
          length_indice = offset_indice;


      // Element buffer
      d.isoband[i].elementbuffer = new_glgrib_opengl_buffer_ptr 
                                    (length_indice * sizeof (unsigned int));

      {
        unsigned int * indice = (unsigned int *)d.isoband[i].elementbuffer->map ();

        // Pack all indices into the buffer, after adding an offset
#pragma omp parallel for
        for (int ith = 0; ith < nth; ith++)
          {
            int offset = isomake[ith][i].offset_lonlat / 2;
            for (int j = 0; j < isomake[ith][i].length_indice; j++)
              indice[isomake[ith][i].offset_indice+j] = 
                isomake[ith][i].indice[j] + offset;
          }

        indice = NULL;
        d.isoband[i].elementbuffer->unmap ();
      }


      // Coordinate buffer
      d.isoband[i].vertexbuffer  = new_glgrib_opengl_buffer_ptr 
                                    (length_lonlat * sizeof (float));

      {
        float * lonlat = (float *)d.isoband[i].vertexbuffer->map ();

        // Pack all lon/lat pairs into the buffer
#pragma omp parallel for
        for (int ith = 0; ith < nth; ith++)
          {
            for (int j = 0; j < isomake[ith][i].length_lonlat; j++)
              lonlat[isomake[ith][i].offset_lonlat+j] = 
                isomake[ith][i].lonlat[j];
          }

        lonlat = NULL;
        d.isoband[i].vertexbuffer->unmap ();
      }

      d.isoband[i].size = length_indice;

    }

  setupVertexAttributes ();

  if (opts.no_value_pointer.on)
    values.push_back (new_glgrib_field_float_buffer_ptr ((float*)NULL));
  else
    values.push_back (data);

  setReady ();

}

void glgrib_field_isofill::render (const glgrib_view & view, const glgrib_options_light & light) const
{

  float scale0[3] = {opts.scale, opts.scale, opts.scale};
  const glgrib_palette & p = palette;

  if (opts.scalar.wireframe.on)
    glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);
  

  glgrib_program * program1 = glgrib_program::load (glgrib_program::ISOFILL1);
  program1->use ();

  view.setMVP (program1);
  program1->set3fv ("scale0", scale0);
  palette.setRGBA255 (program1->programID);

  glBindVertexArray (d.VertexArrayID);

  geometry->renderTriangles ();
  glBindVertexArray (0);
  view.delMVP (program1);

  glgrib_program * program2 = glgrib_program::load (glgrib_program::ISOFILL2);
  program2->use ();

  view.setMVP (program2);
  program2->set3fv ("scale0", scale0);

  for (int i = 0; i < d.isoband.size (); i++)
    {
      float color0[4] = {d.isoband[i].color.r/255.0f, 
                         d.isoband[i].color.g/255.0f, 
                         d.isoband[i].color.b/255.0f,
                         d.isoband[i].color.a/255.0f};


      glBindVertexArray (d.isoband[i].VertexArrayID);
      program2->set4fv ("color0", color0);
      glDrawElements (GL_TRIANGLES, d.isoband[i].size, GL_UNSIGNED_INT, NULL);
      glBindVertexArray (0);
    }


  view.delMVP (program2);

  if (opts.scalar.wireframe.on)
    glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);

}

glgrib_field_isofill::~glgrib_field_isofill ()
{
}


