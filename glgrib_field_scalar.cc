#include "glgrib_field_scalar.h"
#include "glgrib_load.h"
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
      cleanup ();
      operator= (field);
    }
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
      cleanup ();
      if (field.isReady ())
        {
          glgrib_field::operator= (field);
          setupVertexAttributes ();
          setReady ();
        }
    }
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
}

void glgrib_field_scalar::init (const glgrib_options_field & opts, int slot)
{
  unsigned char * col;

  float * data;
  glgrib_field_metadata meta1;
  glgrib_load (opts.path[slot], &data, &meta1);
  meta.push_back (meta1);

  dopts.scale = opts.scale[slot];

  if (opts.palette[slot] == "default")
    dopts.palette = get_palette_by_meta (meta1);
  else
    dopts.palette = get_palette_by_name (opts.palette[slot]);

  geometry = glgrib_geometry_load (opts.path[slot]);

  numberOfColors = 1;

  col = (unsigned char *)malloc (numberOfColors * geometry->numberOfPoints * sizeof (unsigned char));

  for (int i = 0; i < geometry->numberOfPoints; i++)
    if (data[i] == meta1.valmis)
      col[i] = 0;
    else
      col[i] = 1 + (int)(254 * (data[i] - meta1.valmin)/(meta1.valmax - meta1.valmin));

  colorbuffer = new_glgrib_opengl_buffer_ptr (numberOfColors * geometry->numberOfPoints * sizeof (unsigned char), col);

  free (col);

  setupVertexAttributes ();

  if (opts.no_value_pointer)
    {
      values.push_back (NULL);
      free (data);
    }
  else
    {
      values.push_back (new_glgrib_field_float_buffer_ptr (data));
    }


  setReady ();
}

void glgrib_field_scalar::render (const glgrib_view & view, const glgrib_options_light & light) const
{
  glgrib_program * program = glgrib_program_load (glgrib_program::GRADIENT_FLAT_SCALE_SCALAR);
  program->use ();
  float scale0[3] = {dopts.scale, dopts.scale, dopts.scale};
  const glgrib_palette & p = dopts.palette;

  view.setMVP (program);
  program->setLight (light);
  p.setRGBA255 (program->programID);

  program->set3fv ("scale0", scale0);
  program->set1f ("valmin", getNormedMinValue ());
  program->set1f ("valmax", getNormedMaxValue ());

  float palmax = p.hasMax () ? p.getMax () : getNormedMaxValue ();
  float palmin = p.hasMin () ? p.getMin () : getNormedMinValue ();

  program->set1f ("palmin", palmin);
  program->set1f ("palmax", palmax);

  glgrib_field::render (view, light);
}

glgrib_field_scalar::~glgrib_field_scalar ()
{
}


