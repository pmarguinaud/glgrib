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
	 values = NULL;
         ready_ = false;
       }
    }
}

void glgrib_field::init (const glgrib_options_field & opts, int slot)
{
  unsigned char * col;

  float * data;
  glgrib_load (opts.path[slot], &data, &meta);

  dopts.scale = opts.scale[slot];

  if (opts.palette[slot] == "default")
    dopts.palette = get_palette_by_meta (meta);
  else
    dopts.palette = get_palette_by_name (opts.palette[slot]);

  geometry = glgrib_geometry_load (opts.path[slot]);

  numberOfColors = 1;

  col = (unsigned char *)malloc (numberOfColors * geometry->numberOfPoints * sizeof (unsigned char));

  for (int i = 0; i < geometry->numberOfPoints; i++)
    if (data[i] == meta.valmis)
      col[i] = 0;
    else
      col[i] = 1 + (int)(254 * (data[i] - meta.valmin)/(meta.valmax - meta.valmin));

  colorbuffer = new_glgrib_opengl_buffer_ptr (numberOfColors * geometry->numberOfPoints * sizeof (unsigned char), col);

  def_from_vertexbuffer_col_elementbuffer (colorbuffer, geometry);

  free (col);

  values = NULL;

  if (opts.no_value_pointer)
    free (data);
  else
    values = new_glgrib_field_float_buffer_ptr (data);


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
  glUniform1f (glGetUniformLocation (program->programID, "valmin"), meta.valmin);
  glUniform1f (glGetUniformLocation (program->programID, "valmax"), meta.valmax);

  float palmax = p.hasMax () ? p.getMax () : meta.valmax;
  float palmin = p.hasMin () ? p.getMin () : meta.valmin;

  glUniform1f (glGetUniformLocation (program->programID, "palmin"), palmin);
  glUniform1f (glGetUniformLocation (program->programID, "palmax"), palmax);

  glgrib_world::render (view);
}

glgrib_field::~glgrib_field ()
{
}


