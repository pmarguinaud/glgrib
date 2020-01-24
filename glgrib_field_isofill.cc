#include "glgrib_field_isofill.h"
#include "glgrib_trigonometry.h"
#include "glgrib_program.h"
#include "glgrib_palette.h"

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <algorithm>

#include <glm/glm.hpp>


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
    for (int i = 0; i < d.isoband.size (); i++)
      {
        glDeleteVertexArrays (1, &d.isoband[i].VertexArrayID1);
        glDeleteVertexArrays (1, &d.isoband[i].VertexArrayID2);
      }
  glgrib_field::clear ();
}

void glgrib_field_isofill::setupVertexAttributes ()
{
  numberOfPoints = geometry->getNumberOfPoints ();
  numberOfTriangles = geometry->getNumberOfTriangles ();

  for (int i = 0; i < d.isoband.size (); i++)
    {
      // New triangles
      glGenVertexArrays (1, &d.isoband[i].VertexArrayID1);
      glBindVertexArray (d.isoband[i].VertexArrayID1);

      // Elements
      d.isoband[i].elementbuffer1->bind (GL_ELEMENT_ARRAY_BUFFER);

      // Coordinates
      geometry->bindCoordinates (0);
      glEnableVertexAttribArray (0);
      glVertexAttribPointer (0, 2, GL_FLOAT, GL_FALSE, 0, NULL);

      glBindVertexArray (0); 

      // Triangles from original geometry
      glGenVertexArrays (1, &d.isoband[i].VertexArrayID2);
      glBindVertexArray (d.isoband[i].VertexArrayID2);
  
      // Elements
      d.isoband[i].elementbuffer2->bind (GL_ELEMENT_ARRAY_BUFFER);
      d.isoband[i].vertexbuffer2->bind (GL_ARRAY_BUFFER);
  
      glEnableVertexAttribArray (0);
      glVertexAttribPointer (0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
  
      glBindVertexArray (0); 
    }
}


void glgrib_field_isofill::processTriangle2 (const float v[3], const glm::vec3 xyz[3],
                                             const std::vector<float> & levels)
{

  processTriangle2_ctx_t ctx;

  for (int i = 0; i < 3; i++)
    ctx.lonlat[i] = xyz2lonlat (xyz[i]);
  ctx.v = v;


  auto close = [&ctx] (isoband_t & ib)
  {
    int i = std::max_element (ctx.v, ctx.v + 3) - ctx.v;
    int j = (i + 1) % 3;
    int k = (j + 1) % 3;
    if (ctx.I == i)
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

      isoband_t & ib = d.isoband[ll];

      bool b[3];
      for (int i = 0; i < 3; i++)
        b[i] = lev <= v[i];

//    printf (" ll = %8d, lev = %12.2f, b = %d, %d, %d\n", ll, lev, b[0], b[1], b[2]);

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
              
	      int ind0 = ib.lonlat2.size () / 2;

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


  close (d.isoband.back ());
}

void glgrib_field_isofill::processTriangle1 (const float * val, int it, const std::vector<float> & levels)
{
  int jglo[3];
  
  geometry->getTriangleVertices (it, jglo);

  float v[3] = {val[jglo[0]], val[jglo[1]], val[jglo[2]]};

  if ((v[0] <= levels.front ()) && (v[1] <= levels.front ()) && (v[2] <= levels.front ()))
    {
      d.isoband.front ().ind1.push_back (jglo[0]);
      d.isoband.front ().ind1.push_back (jglo[1]);
      d.isoband.front ().ind1.push_back (jglo[2]);
      return;
    }

  if ((v[0] >= levels.back  ()) && (v[1] >= levels.back  ()) && (v[2] >= levels.back  ()))
    {
      d.isoband.back ().ind1.push_back (jglo[0]);
      d.isoband.back ().ind1.push_back (jglo[1]);
      d.isoband.back ().ind1.push_back (jglo[2]);
      return;
    }

  for (int i = 0; i < levels.size ()-1; i++)
    if (((v[0] >= levels[i+0]) && (v[1] >= levels[i+0]) && (v[2] >= levels[i+0]))
     && ((v[0] <= levels[i+1]) && (v[1] <= levels[i+1]) && (v[2] <= levels[i+1])))
      {
        d.isoband[i+1].ind1.push_back (jglo[0]);
        d.isoband[i+1].ind1.push_back (jglo[1]);
        d.isoband[i+1].ind1.push_back (jglo[2]);
        return;
      }

  glm::vec3 xyz[3];

  for (int i = 0; i < 3; i++)
    {
      float lon, lat;
      geometry->index2latlon (jglo[i], &lat, &lon);
      xyz[i] = lonlat2xyz (glm::vec2 (lon, lat));
    }

//if ((it == 44540) || (it == 44541))
  {
//printf (" jglo = %d, %d, %d\n", jglo[0], jglo[1], jglo[2]);
//printf (" v    = %12.2f, %12.2f, %12.2f\n", v[0], v[1], v[2]);
  processTriangle2 (v, xyz, levels);
  }


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
      float min = opts.isofill.min == glgrib_options_isofill::defaultMin ? meta1.valmin : opts.isofill.min;
      float max = opts.isofill.max == glgrib_options_isofill::defaultMax ? meta1.valmax : opts.isofill.max;
      for (int i = 0; i < opts.isofill.number; i++)
        levels.push_back (min + (i + 1) * (max - min) / (opts.isofill.number + 1));
    }

  for (int i = 0; i < levels.size (); i++)
    printf (" %8d > %12.2f\n", i, levels[i]);

  d.isoband.resize (levels.size () + 1);

  float * val = data->data ();

  int nt = geometry->getNumberOfTriangles ();
  std::cout << " nt = " << nt << std::endl;

  for (int it = 0; it < nt; it++)
    processTriangle1 (val, it, levels);

  val = NULL;

  int count = 0;
  for (int i = 0; i < d.isoband.size (); i++)
    {
      float v;
      if (i == 0)
        v = levels.front () * 0.99f - 1.0f;
      else if (i == d.isoband.size () - 1)
        v = levels.back  () * 1.01f + 1.0f;
      else
        v = (levels[i-1] + levels[i+0]) / 2.0f;

      d.isoband[i].color = palette.getColor (v);

      d.isoband[i].elementbuffer1 = new_glgrib_opengl_buffer_ptr 
                                    (d.isoband[i].ind1.size () * sizeof (unsigned int), 
                                     d.isoband[i].ind1.data ());

      d.isoband[i].size1 = d.isoband[i].ind1.size ();

      printf (" %12.2f | %8d | %8d | #%2.2x%2.2x%2.2x%2.2x\n", 
              v, i, d.isoband[i].ind1.size () / 3, 
              d.isoband[i].color.r, d.isoband[i].color.g,
              d.isoband[i].color.b, d.isoband[i].color.a);

      count += d.isoband[i].ind1.size () / 3;

      d.isoband[i].ind1.clear ();

      d.isoband[i].elementbuffer2 = new_glgrib_opengl_buffer_ptr 
                                    (d.isoband[i].ind2.size () * sizeof (unsigned int), 
                                     d.isoband[i].ind2.data ());

      d.isoband[i].vertexbuffer2  = new_glgrib_opengl_buffer_ptr 
                                    (d.isoband[i].lonlat2.size () * sizeof (float), 
                                     d.isoband[i].lonlat2.data ());

      d.isoband[i].size2 = d.isoband[i].ind2.size ();

    }

  printf (" count = %d\n", count);

  setupVertexAttributes ();

  if (opts.no_value_pointer.on)
    {
      values.push_back (new_glgrib_field_float_buffer_ptr ((float*)NULL));
    }
  else
    {
      values.push_back (data);
    }


  setReady ();
}

void glgrib_field_isofill::render (const glgrib_view & view, const glgrib_options_light & light) const
{
  glgrib_program * program = glgrib_program::load (glgrib_program::ISOFILL);
  program->use ();
  float scale0[3] = {opts.scale, opts.scale, opts.scale};
  const glgrib_palette & p = palette;

  view.setMVP (program);
  program->set3fv ("scale0", scale0);
  program->set1f ("height_scale", opts.geometry.height.scale);

  for (int i = 0; i < d.isoband.size (); i++)
    {
      float color0[4] = {d.isoband[i].color.r/255.0f, 
                         d.isoband[i].color.g/255.0f, 
                         d.isoband[i].color.b/255.0f,
                         d.isoband[i].color.a/255.0f};


      glBindVertexArray (d.isoband[i].VertexArrayID1);
      program->set4fv ("color0", color0);
      glDrawElements (GL_TRIANGLES, d.isoband[i].size1, GL_UNSIGNED_INT, NULL);
      glBindVertexArray (0);


      glBindVertexArray (d.isoband[i].VertexArrayID2);
      program->set4fv ("color0", color0);
      glDrawElements (GL_TRIANGLES, d.isoband[i].size2, GL_UNSIGNED_INT, NULL);
      glBindVertexArray (0);
    }


  view.delMVP (program);

}

glgrib_field_isofill::~glgrib_field_isofill ()
{
}


