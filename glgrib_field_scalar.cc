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
  glVertexAttribPointer (1, 1, GL_UNSIGNED_BYTE, GL_TRUE, sizeof (unsigned char), NULL); 

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
  glVertexAttribPointer (1, 1, GL_UNSIGNED_BYTE, GL_TRUE, sizeof (unsigned char), NULL); 
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

void glgrib_field_scalar::setup (glgrib_loader * ld, const glgrib_options_field & o, float slot)
{
  opts = o;

  const int jglo0 = 60486;

  glgrib_field_metadata meta1;

  glgrib_field_float_buffer_ptr data;
  ld->load (&data, opts.path, opts.geometry, slot, &meta1, 1, 0, opts.diff.on);
  meta.push_back (meta1);

  palette = glgrib_palette::create (opts.palette, getNormedMinValue (), getNormedMaxValue ());

  geometry = glgrib_geometry::load (ld, opts.path[0], opts.geometry);

  if (opts.hilo.on)
    setupHilo (data);

  colorbuffer = new_glgrib_opengl_buffer_ptr (geometry->getNumberOfPoints () * sizeof (unsigned char));
  unsigned char * col = (unsigned char *)colorbuffer->map ();
  pack8 (data->data (), geometry->getNumberOfPoints (), meta1.valmin, meta1.valmax, meta1.valmis, col);

  float z = meta1.valmin + (meta1.valmax - meta1.valmin) * (col[jglo0] - 1.0f) / 254.0f;
  std::cout << " data = " << (*data)[jglo0] << " " << (int)col[jglo0]  << " "
	  << z
	  << std::endl;

  std::cout << " data => " << palette.getColor ((*data)[jglo0]) << std::endl;
  std::cout << " z => " << palette.getColor (z) << std::endl;

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

  renderHilo (view);

}

glgrib_field_scalar::~glgrib_field_scalar ()
{
}


