#include "glgrib_field_scalar.h"
#include "glgrib_program.h"
#include "glgrib_palette.h"

#include <stdlib.h>
#include <stdio.h>
#include <iostream>

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
  numberOfPoints = geometry->numberOfPoints;
  numberOfTriangles = geometry->numberOfTriangles;

  glGenVertexArrays (1, &VertexArrayID);
  glBindVertexArray (VertexArrayID);

  geometry->vertexbuffer->bind (GL_ARRAY_BUFFER);
  glEnableVertexAttribArray (0); 
  glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 0, NULL); 
  
  colorbuffer->bind (GL_ARRAY_BUFFER);
  glEnableVertexAttribArray (1); 
  glVertexAttribPointer (1, numberOfColors, GL_UNSIGNED_BYTE, GL_TRUE, numberOfColors * sizeof (unsigned char), NULL); 

  geometry->elementbuffer->bind (GL_ELEMENT_ARRAY_BUFFER);

  glBindVertexArray (0); 


  glGenVertexArrays (1, &VertexArrayIDpoints);
  glBindVertexArray (VertexArrayIDpoints);

  geometry->vertexbuffer->bind (GL_ARRAY_BUFFER);
  glEnableVertexAttribArray (0); 
  glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 0, NULL); 
  glVertexAttribDivisor (0, 1);
  
  colorbuffer->bind (GL_ARRAY_BUFFER);
  glEnableVertexAttribArray (1); 
  glVertexAttribPointer (1, numberOfColors, GL_UNSIGNED_BYTE, GL_TRUE, numberOfColors * sizeof (unsigned char), NULL); 
  glVertexAttribDivisor (1, 1);

  glBindVertexArray (0); 
}

void glgrib_field_scalar::setup (glgrib_loader * ld, const glgrib_options_field & o, float slot)
{
  opts = o;

  unsigned char * col;

  glgrib_field_metadata meta1;

  glgrib_field_float_buffer_ptr data;
  ld->load (&data, opts.path, opts.geometry, slot, &meta1, 1, 0, opts.diff.on);
  meta.push_back (meta1);

  palette = glgrib_palette::create (opts.palette, getNormedMinValue (), getNormedMaxValue ());

  geometry = glgrib_geometry::load (ld, opts.path[0], opts.geometry);

  numberOfColors = 1;

  col = (unsigned char *)malloc (numberOfColors * geometry->numberOfPoints * sizeof (unsigned char));

  for (int i = 0; i < geometry->numberOfPoints; i++)
    if ((*data)[i] == meta1.valmis)
      col[i] = 0;
    else
      col[i] = 1 + (int)(254 * ((*data)[i] - meta1.valmin)/(meta1.valmax - meta1.valmin));

  colorbuffer = new_glgrib_opengl_buffer_ptr (numberOfColors * geometry->numberOfPoints * sizeof (unsigned char), col);

  free (col);

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

  glgrib_program * program = glgrib_program_load (opts.scalar.points.on 
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
      if (geometry->ind_strip_size)
        {
          glEnable (GL_PRIMITIVE_RESTART);
          glPrimitiveRestartIndex (0xffffffff);
          glDrawElements (GL_TRIANGLE_STRIP, geometry->ind_strip_size, GL_UNSIGNED_INT, NULL);
          glDisable (GL_PRIMITIVE_RESTART);
        }
      else
        {
          glDrawElements (GL_TRIANGLES, 3 * numberOfTriangles, GL_UNSIGNED_INT, NULL);
        }
      
      glBindVertexArray (0);

      if (opts.scalar.wireframe.on)
        glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
    }

  view.delMVP (program);

}

glgrib_field_scalar::~glgrib_field_scalar ()
{
}


