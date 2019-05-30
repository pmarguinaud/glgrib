#include "glgrib_field.h"
#include "glgrib_load.h"
#include "glgrib_program.h"
#include "glgrib_palette.h"

#include <stdlib.h>
#include <stdio.h>
#include <iostream>

glgrib_field_float_buffer::glgrib_field_float_buffer (size_t size)
{
  data_ = new float[size];
}

glgrib_field_float_buffer::glgrib_field_float_buffer (float * data)
{
  data_ = data;
}

glgrib_field_float_buffer::~glgrib_field_float_buffer ()
{
  if (data_)
    delete [] data_;
  data_ = NULL;
}

glgrib_field_float_buffer_ptr new_glgrib_field_float_buffer_ptr (size_t size)
{
  return std::make_shared<glgrib_field_float_buffer>(size);
}

glgrib_field_float_buffer_ptr new_glgrib_field_float_buffer_ptr (float * data)
{
  return std::make_shared<glgrib_field_float_buffer>(data);
}

glgrib_field::glgrib_field (const glgrib_field & field)
{
  if (field.isReady ())
    {
      // Cleanup already existing VAOs
      cleanup ();
      operator= (field);
    }
}

glgrib_field * glgrib_field::clone () const
{
  if (this == NULL)
    return NULL;
  glgrib_field * fld = new glgrib_field ();
  *fld = *this;
  return fld;
}

glgrib_field & glgrib_field::operator= (const glgrib_field & field)
{
  if (this != &field)
    {
      if (field.isReady ())
        {
          glgrib_world::operator= (field);
          ready_ = false;
	  meta   = field.meta;
          values = field.values;
          dopts  = field.dopts;
          def_from_vertexbuffer_col_elementbuffer (colorbuffer, geometry);
          setReady ();
        }
      else
       {
         cleanup ();
	 values.clear ();
         ready_ = false;
       }
    }
}

void glgrib_field::init (const glgrib_options_field & opts, int slot)
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

  values.clear ();

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

glgrib_field_display_options::glgrib_field_display_options ()
{
    palette = palette_cloud_auto;
    palette = palette_cold_hot;
}

void glgrib_field::render (const glgrib_view * view) const
{
  const glgrib_program * program = get_program (); 
  float scale0[3] = {dopts.scale, dopts.scale, dopts.scale};
  const glgrib_palette & p = dopts.palette;

  p.setRGBA255 (program->programID);

  glUniform3fv (glGetUniformLocation (program->programID, "scale0"), 1, scale0);
  glUniform1f (glGetUniformLocation (program->programID, "valmin"), getNormedMinValue ());
  glUniform1f (glGetUniformLocation (program->programID, "valmax"), getNormedMaxValue ());

  float palmax = p.hasMax () ? p.getMax () : getNormedMinValue ();
  float palmin = p.hasMin () ? p.getMin () : getNormedMaxValue ();

  glUniform1f (glGetUniformLocation (program->programID, "palmin"), palmin);
  glUniform1f (glGetUniformLocation (program->programID, "palmax"), palmax);

  glgrib_world::render (view);
}

glgrib_field::~glgrib_field ()
{
}


