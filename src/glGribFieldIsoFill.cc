#include "glGribFieldIsoFill.h"
#include "glGribTrigonometry.h"
#include "glGribProgram.h"
#include "glGribPalette.h"

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <algorithm>

#include <glm/glm.hpp>
#include <omp.h>


glGrib::FieldIsoFill * glGrib::FieldIsoFill::clone () const
{
  glGrib::FieldIsoFill * fld = new glGrib::FieldIsoFill ();
  *fld = *this;
  return fld;
}

void glGrib::FieldIsoFill::clear ()
{
  if (isReady ()) 
    {
      d.VAID.clear ();
      for (auto & ib : d.isoband)
        ib.clear ();
    }
  glGrib::Field::clear ();
}

void glGrib::FieldIsoFill::isoband_t::setupVertexAttributes () const
{
  glGrib::Program * program = glGrib::Program::load ("ISOFILL2");

  // Elements
  d.elementbuffer->bind (GL_ELEMENT_ARRAY_BUFFER);

  // Values
  d.vertexbuffer->bind (GL_ARRAY_BUFFER);
  auto attr = program->getAttributeLocation ("vertexLonLat");
  glEnableVertexAttribArray (attr);
  glVertexAttribPointer (attr, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
}

void glGrib::FieldIsoFill::setupVertexAttributes () const
{
  glGrib::Program * program = glGrib::Program::load ("ISOFILL1");

  // Triangles from original geometry

  geometry->bindTriangles ();

  // Coordinates
  auto pattr = program->getAttributeLocation ("vertexLonLat");
  geometry->bindCoordinates (pattr);

  // Values
  auto vattr = program->getAttributeLocation ("vertexColInd");
  d.colorbuffer->bind (GL_ARRAY_BUFFER);
  glEnableVertexAttribArray (vattr);
  glVertexAttribPointer (vattr, 1, GL_UNSIGNED_BYTE, GL_FALSE, 0,  nullptr);
}

namespace
{

class isoband_maker_t
{
public:

  int offset_indice, length_indice;
  int offset_lonlat, length_lonlat;

  std::vector<unsigned int> indice;
  std::vector<float> lonlat;

  void pushLonLat (const glm::vec2 & lonlat_)
  {
    lonlat.push_back (lonlat_.x);
    lonlat.push_back (lonlat_.y);
  }

  template <typename T, typename... Args>
  void pushLonLat (const T & lonlat, Args... args)
  {
    pushLonLat (lonlat);
    pushLonLat (args...);
  }

  void pushIndice (int ind)
  {
    indice.push_back (ind);
  }

  template <typename T, typename... Args>
  void pushIndice (T ind, Args... args)
  {
    pushIndice (ind);
    pushIndice (args...);
  }

  void quad (const glm::vec2 & lonlata, const glm::vec2 & lonlatb, 
             const glm::vec2 & lonlatc, const glm::vec2 & lonlatd,
             bool direct)
  {
    int ind0 = lonlat.size () / 2;

    pushLonLat (lonlata, lonlatb, 
                 lonlatc, lonlatd);
    
    int ord[6] = {0, 1, 2, 0, 2, 3};

    if (! direct)
      {
        std::swap (ord[0], ord[1]);
        std::swap (ord[3], ord[4]);
      }

    pushIndice (ind0+ord[0], ind0+ord[1], 
                 ind0+ord[2], ind0+ord[3], 
                 ind0+ord[4], ind0+ord[5]);

  }

  void tri (const glm::vec2 & lonlata, 
            const glm::vec2 & lonlatb, 
            const glm::vec2 & lonlatc)
  {
    int ind0 = lonlat.size () / 2;

    pushLonLat (lonlata, lonlatb, lonlatc);
    pushIndice (ind0+0, ind0+1, ind0+2);
  }

  void penta (const glm::vec2 & lonlata, const glm::vec2 & lonlatb, 
        	const glm::vec2 & lonlatc, const glm::vec2 & lonlatd,
        	const glm::vec2 & lonlate)
  {
    int ind0 = lonlat.size () / 2;

    pushLonLat (lonlata, lonlatb, lonlatc,
                 lonlatd, lonlate);

    pushIndice (ind0+0, ind0+1, ind0+2,
                 ind0+0, ind0+2, ind0+3,
                 ind0+0, ind0+3, ind0+4);
  }
 
};

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
    const float * v = nullptr;
  };

  processTriangle2_ctx_t ctx;

  for (int i = 0; i < 3; i++)
    ctx.lonlat[i] = glGrib::xyz2lonlat (xyz[i]);
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

  for (size_t ll = 0; ll < levels.size (); ll++)
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
	      glm::vec2 lonlat_j = glGrib::xyz2lonlat (xyz_j),
	                lonlat_k = glGrib::xyz2lonlat (xyz_k);
              
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

void processTriangle1 (std::vector<isoband_maker_t> * isomake, 
                       glGrib::const_GeometryPtr geometry,
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

  for (size_t i = 1; i < levels.size (); i++)
    if (((v[0] > levels[i-1]) && (v[1] > levels[i-1]) && (v[2] > levels[i-1]))
     && ((v[0] < levels[i+0]) && (v[1] < levels[i+0]) && (v[2] < levels[i+0])))
      return;

  // At this point, we know the triangle will not have an homogeneous color;
  // we will have to split it into subtriangles

  glm::vec3 xyz[3];

  for (int i = 0; i < 3; i++)
    {
      float lon, lat;
      geometry->index2latlon (jglo[i], &lat, &lon);
      xyz[i] = glGrib::lonlat2xyz (glm::vec2 (lon, lat));
    }

  processTriangle2 (isomake, v, xyz, levels);
}

}

void glGrib::FieldIsoFill::setup (glGrib::Loader * ld, const glGrib::OptionsField & o, float slot)
{
  opts = o;

  glGrib::FieldMetadata meta1;
  glGrib::FieldFloatBufferPtr data;
  ld->load (&data, opts.path, opts.geometry, slot, &meta1, 1, 0, opts.diff.on);
  meta.push_back (meta1);

  palette = glGrib::Palette (opts.palette, getNormedMinValue (), getNormedMaxValue ());

  geometry = glGrib::Geometry::load (ld, opts.path[int (slot)], opts.geometry);

  int size = geometry->size ();

  std::vector<float> levels = opts.isofill.levels;

  if (levels.size () == 0)
    {
      float min = opts.isofill.min == glGrib::OptionsIsofill::defaultMin ()
                ? meta1.valmin : opts.isofill.min;
      float max = opts.isofill.max == glGrib::OptionsIsofill::defaultMax ()
                ? meta1.valmax : opts.isofill.max;
      for (int i = 0; i < opts.isofill.number; i++)
        levels.push_back (min + (i + 1) * (max - min) / (opts.isofill.number + 1));
    }

  d.isoband.resize (levels.size () + 1);
 

  int nth = omp_get_max_threads ();
  std::vector<isoband_maker_t> isomake[nth];

  for (int ith = 0; ith < nth; ith++)
    isomake[ith].resize (d.isoband.size ());


  std::vector<int> colorIndex (d.isoband.size ());

  for (size_t i = 0; i < d.isoband.size (); i++)
    {
      float v;
      if (i == 0)
        v = levels.front () * 0.99f - 1.0f;
      else if (static_cast<int> (i) == static_cast<int> (d.isoband.size ()) - 1)
        v = levels.back  () * 1.01f + 1.0f;
      else
        v = (levels[i-1] + levels[i+0]) / 2.0f;

      d.isoband[i].d.color = palette.getColor      (v);
      colorIndex[i]        = palette.getColorIndex (v);
    }


  float * val = data->data ();

#pragma omp parallel for
  for (int it = 0; it < geometry->getNumberOfTriangles (); it++)
    {
      int ith = omp_get_thread_num ();
      processTriangle1 (&isomake[ith], geometry, val, it, levels);
    }


  {
    T * color = new T[size];

#pragma omp parallel for
    for (int i = 0; i < size; i++)
      {
        float v = val[i];
        if (v < levels.front ())
          color[i] = colorIndex.front ();
        else if (v > levels.back ())
          color[i] = colorIndex.back  ();
        else
          for (size_t j = 1; j < levels.size (); j++)
            if ((levels[j-1] < v) && (v < levels[j+0]))
              {
                color[i] = colorIndex[j+0];
                break;
              }
      }

    d.colorbuffer  = glGrib::OpenGLBufferPtr<T> (size, color);
    
    delete [] color;
  }

  val = nullptr;

  for (size_t i = 0; i < d.isoband.size (); i++)
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
      d.isoband[i].d.elementbuffer = glGrib::OpenGLBufferPtr<unsigned int> (length_indice);

      {
        auto indice = d.isoband[i].d.elementbuffer->map ();
        // Pack all indices into the buffer, after adding an offset
#pragma omp parallel for
        for (int ith = 0; ith < nth; ith++)
          {
            int offset = isomake[ith][i].offset_lonlat / 2;
            for (int j = 0; j < isomake[ith][i].length_indice; j++)
              indice[isomake[ith][i].offset_indice+j] = 
                isomake[ith][i].indice[j] + offset;
          }
      }


      // Coordinate buffer
      d.isoband[i].d.vertexbuffer  = glGrib::OpenGLBufferPtr<float> (length_lonlat);

      {
        auto lonlat = d.isoband[i].d.vertexbuffer->map ();

        // Pack all lon/lat pairs into the buffer
#pragma omp parallel for
        for (int ith = 0; ith < nth; ith++)
          {
            for (int j = 0; j < isomake[ith][i].length_lonlat; j++)
              lonlat[isomake[ith][i].offset_lonlat+j] = 
                isomake[ith][i].lonlat[j];
          }
      }

      d.isoband[i].d.size = length_indice;

    }

  if (opts.no_value_pointer.on)
    data = glGrib::FieldFloatBufferPtr (0);

  values.push_back (data);

  setReady ();

}

void glGrib::FieldIsoFill::isoband_t::render () const
{
  glGrib::Program * program = glGrib::Program::load ("ISOFILL2");
  program->set ("color0", d.color);
  glDrawElements (GL_TRIANGLES, d.size, GL_UNSIGNED_INT, nullptr);
}

void glGrib::FieldIsoFill::render (const glGrib::View & view, 
                                   const glGrib::OptionsLight & light) 
const
{
  if (opts.scalar.wireframe.on)
    glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);
  

  glGrib::Program * program = glGrib::Program::load ("ISOFILL1");
  program->use ();

  view.setMVP (program);
  program->set ("scale0", opts.scale);
  palette.set (program);

  d.VAID.bind ();
  geometry->renderTriangles ();
  d.VAID.unbind ();

  view.delMVP (program);

  glGrib::Program * program2 = glGrib::Program::load ("ISOFILL2");
  program2->use ();

  view.setMVP (program2);
  program2->set ("scale0", opts.scale);

  for (const auto & ib : d.isoband)
    ib.VAID.render ();

  view.delMVP (program2);

  if (opts.scalar.wireframe.on)
    glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);

}


