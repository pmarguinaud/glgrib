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
          ready_ = false;
          def_from_vertexbuffer_col_elementbuffer (colorbuffer, geometry);
          setReady ();
        }
      else
       {
         ready_ = false;
       }
    }
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

  def_from_vertexbuffer_col_elementbuffer (colorbuffer, geometry);

  free (col);

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

void glgrib_field_scalar::render (const glgrib_view * view) const
{
  const glgrib_program * program = get_program (); 
  float scale0[3] = {dopts.scale, dopts.scale, dopts.scale};
  const glgrib_palette & p = dopts.palette;

  p.setRGBA255 (program->programID);

  glUniform3fv (glGetUniformLocation (program->programID, "scale0"), 1, scale0);
  glUniform1f (glGetUniformLocation (program->programID, "valmin"), getNormedMinValue ());
  glUniform1f (glGetUniformLocation (program->programID, "valmax"), getNormedMaxValue ());

  float palmax = p.hasMax () ? p.getMax () : getNormedMaxValue ();
  float palmin = p.hasMin () ? p.getMin () : getNormedMinValue ();

  glUniform1f (glGetUniformLocation (program->programID, "palmin"), palmin);
  glUniform1f (glGetUniformLocation (program->programID, "palmax"), palmax);

  glgrib_field::render (view);
}

glgrib_field_scalar::~glgrib_field_scalar ()
{
}


