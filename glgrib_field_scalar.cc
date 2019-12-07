#include "glgrib_field_scalar.h"
#include "glgrib_program.h"
#include "glgrib_palette.h"

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <algorithm>

glgrib_field_scalar::glgrib_field_scalar (const glgrib_field_scalar & field)
{
  if (field.isReady ())
    {
      // Cleanup already existing VAOs
      clear ();
      operator= (field);
    }
}

void glgrib_field_scalar::clear ()
{
  if (isReady ())
    glDeleteVertexArrays (1, &VertexArrayIDpoints);
  glgrib_field::clear ();
}


glgrib_field_scalar * glgrib_field_scalar::clone () const
{
  if (this == NULL)
    return NULL;
  glgrib_field_scalar * fld = new glgrib_field_scalar ();
  *fld = *this;
  return fld;
}

glgrib_field_scalar & glgrib_field_scalar::operator= (const glgrib_field_scalar & field)
{
  if (this != &field)
    {
      clear ();
      if (field.isReady ())
        {
          glgrib_field::operator= (field);
          setupVertexAttributes ();
          setReady ();
        }
    }
  return *this;
}

void glgrib_field_scalar::setupVertexAttributes ()
{
  numberOfPoints = geometry->getNumberOfPoints ();
  numberOfTriangles = geometry->getNumberOfTriangles ();

  // Colored field
  glGenVertexArrays (1, &VertexArrayID);
  glBindVertexArray (VertexArrayID);

  geometry->bindCoordinates (0);
  
  colorbuffer->bind (GL_ARRAY_BUFFER);
  glEnableVertexAttribArray (1); 
  glVertexAttribPointer (1, numberOfColors, GL_UNSIGNED_BYTE, GL_TRUE, numberOfColors * sizeof (unsigned char), NULL); 

  geometry->bindTriangles ();

  bindHeight (2);

  glBindVertexArray (0); 


  // Points
  glGenVertexArrays (1, &VertexArrayIDpoints);
  glBindVertexArray (VertexArrayIDpoints);

  geometry->bindCoordinates (0);
  glVertexAttribDivisor (0, 1);
  
  colorbuffer->bind (GL_ARRAY_BUFFER);
  glEnableVertexAttribArray (1); 
  glVertexAttribPointer (1, numberOfColors, GL_UNSIGNED_BYTE, GL_TRUE, numberOfColors * sizeof (unsigned char), NULL); 
  glVertexAttribDivisor (1, 1);

  if (heightbuffer)
    {
      heightbuffer->bind (GL_ARRAY_BUFFER);
      glEnableVertexAttribArray (2);
      glVertexAttribPointer (2, 1, GL_UNSIGNED_BYTE, GL_TRUE, 0, NULL);
      glVertexAttribDivisor (2, 1);
    }
  else
    {
      glDisableVertexAttribArray (2);
      glVertexAttrib1f (2, 0.0f);
    }

  glBindVertexArray (0); 
}

static
int hilo_count (const_glgrib_geometry_ptr geometry, glgrib_field_float_buffer_ptr data,
                int jglo, int radius)
{
  const float * val = data->data ();
  std::vector<int> neigh;
  std::set<int> seen;
  
  seen.insert (jglo);

  std::set<int> seen1, * s2 = &seen1, seen2, * s1 = &seen2;

  // s1 : last inserted values
  // s2 : values being inserted
  s1->insert (jglo);

  for (int j = 1; j < radius; j++)
    {
      s2->clear ();
      for (std::set<int>::iterator it = s1->begin (); it != s1->end (); it++)
        {
          int jglo0 = *it;
          geometry->getPointNeighbours (jglo0, &neigh);
          for (int i = 0; i < neigh.size (); i++)
            {
              int jglo1 = neigh[i];
              if (seen.find (jglo1) == seen.end ())
                continue;
              if (val[jglo1] > val[jglo0])
                return j-1;
              s2->insert (jglo1);
            }
        }
      for (std::set<int>::iterator it = s2->begin (); it != s2->end (); it++)
        seen.insert (*it);
      std::swap (s1, s2);
    }

  return radius;
}

static
void hilo (const_glgrib_geometry_ptr geometry, glgrib_field_float_buffer_ptr data)
{
  std::vector<int> neigh;
  std::vector<int> jglo_lo, jglo_hi;
  int np = geometry->getNumberOfPoints ();

  const float * val = data->data ();

  int nhi = 0, nlo = 0;
  for (int jglo = 0; jglo < np; jglo++)
    {
      geometry->getPointNeighbours (jglo, &neigh);
      bool hi = true, lo = true;
      for (int i = 0; i < neigh.size (); i++)
        {
          hi = hi && (val[jglo] > val[neigh[i]]);
          lo = lo && (val[jglo] < val[neigh[i]]);
          if ((! hi) && (! lo))
            break;
        }
      if (hi)
        jglo_hi.push_back (jglo);
      if (lo)
        jglo_lo.push_back (jglo);
    }

  auto cmp_lo = [val](int i, int j) { return val[i] <= val[j]; }; 
  std::sort (jglo_lo.begin (), jglo_lo.end (), cmp_lo);
  auto cmp_hi = [val](int i, int j) { return val[i] >= val[j]; }; 
  std::sort (jglo_hi.begin (), jglo_hi.end (), cmp_hi);

  std::cout << " np = " << np  << std::endl;
  std::cout << " lo = " << jglo_lo.size () << std::endl;
  std::cout << " hi = " << jglo_hi.size () << std::endl;
  std::cout << val[jglo_hi[0]] << " " << val[jglo_hi[1]] << std::endl;
  std::cout << val[jglo_lo[0]] << " " << val[jglo_lo[1]] << std::endl;

}

void glgrib_field_scalar::setup (glgrib_loader * ld, const glgrib_options_field & o, float slot)
{
  opts = o;


  glgrib_field_metadata meta1;

  glgrib_field_float_buffer_ptr data;
  ld->load (&data, opts.path, opts.geometry, slot, &meta1, 1, 0, opts.diff.on);
  meta.push_back (meta1);

  palette = glgrib_palette::create (opts.palette, getNormedMinValue (), getNormedMaxValue ());

  geometry = glgrib_geometry::load (ld, opts.path[0], opts.geometry);

  hilo (geometry, data);

  numberOfColors = 1;

  colorbuffer = new_glgrib_opengl_buffer_ptr (numberOfColors * geometry->getNumberOfPoints () * sizeof (unsigned char));
  unsigned char * col = (unsigned char *)colorbuffer->map ();

  for (int i = 0; i < geometry->getNumberOfPoints (); i++)
    if ((*data)[i] == meta1.valmis)
      col[i] = 0;
    else
      col[i] = 1 + (int)(254 * ((*data)[i] - meta1.valmin)/(meta1.valmax - meta1.valmin));

  col = NULL;
  colorbuffer->unmap ();

  loadHeight (colorbuffer, ld);

  setupVertexAttributes ();

  if (opts.no_value_pointer.on)
    values.push_back (new_glgrib_field_float_buffer_ptr ((float*)NULL));
  else
    values.push_back (data);

  setReady ();
}

void glgrib_field_scalar::render (const glgrib_view & view, const glgrib_options_light & light) const
{
  float scale0[3] = {opts.scale, opts.scale, opts.scale};

  glgrib_program * program = glgrib_program::load (opts.scalar.points.on 
                                                ? glgrib_program::SCALAR_POINTS 
                                                : glgrib_program::GRADIENT_FLAT_SCALE_SCALAR);

  program->use ();
  view.setMVP (program);
  program->setLight (light);
  palette.setRGBA255 (program->programID);
  program->set3fv ("scale0", scale0);
  program->set1f ("valmin", getNormedMinValue ());
  program->set1f ("valmax", getNormedMaxValue ());
  program->set1f ("palmin", palette.getMin ());
  program->set1f ("palmax", palette.getMax ());
  program->set1f ("height_scale", opts.geometry.height.scale);
    
  if (opts.scalar.points.on)
    {
      float length = view.pixel_to_dist_at_nadir (10);
    
      program->set1f ("length10", length);
      program->set1f ("pointSiz", opts.scalar.points.size.value);
      program->set1i ("lpointZoo", opts.scalar.points.size.variable.on);
      program->set1i ("factor", opts.scalar.points.size.factor.on);
    
      glBindVertexArray (VertexArrayIDpoints);
    
      unsigned int ind[6] = {0, 1, 2, 2, 3, 0}; 
      glDrawElementsInstanced (GL_TRIANGLES, 6, GL_UNSIGNED_INT, ind, numberOfPoints);
    
      glBindVertexArray (0);
    }
  else
    {
      program->set1i ("smoothed", opts.scalar.smooth.on);
    
      if (opts.scalar.wireframe.on)
        glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);
    
      glBindVertexArray (VertexArrayID);

      geometry->renderTriangles ();
      
      glBindVertexArray (0);

      if (opts.scalar.wireframe.on)
        glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
    }

  view.delMVP (program);

}

glgrib_field_scalar::~glgrib_field_scalar ()
{
}


