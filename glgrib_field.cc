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
  if ((this != &field) && field.isReady ())
    {
      glgrib_world::operator= (field);
      ready_ = false;
      valmis = field.valmis;
      valmin = field.valmin;
      valmax = field.valmax;
      values = field.values;
      def_from_vertexbuffer_col_elementbuffer (colorbuffer, geometry);
      setReady ();
    }
}

void glgrib_field::init (const std::string & field, const glgrib_options & o, const glgrib_geometry_ptr geom)
{
  unsigned char * col;

  geometry = geom;

  numberOfColors = 1;

  float * data;
  glgrib_load (field, &data, &valmin, &valmax, &valmis);
  values = new_glgrib_field_float_buffer_ptr (data);

  col = (unsigned char *)malloc (numberOfColors * geom->numberOfPoints * sizeof (unsigned char));

  for (int i = 0; i < geom->numberOfPoints; i++)
    if (data[i] == valmis)
      col[i] = 0;
    else
      col[i] = 1 + (int)(254 * (data[i] - valmin)/(valmax - valmin));

  colorbuffer = new_glgrib_opengl_buffer_ptr (numberOfColors * geom->numberOfPoints * sizeof (unsigned char), col);

  def_from_vertexbuffer_col_elementbuffer (colorbuffer, geometry);

  free (col);

  setReady ();
}

glgrib_field_display_options::glgrib_field_display_options ()
{
    palette = palette_cloud_auto;
    palette = palette_cold_hot;
}

void glgrib_field::render (const glgrib_view * view, const glgrib_field_display_options & dopts) const
{
  const glgrib_program * program = get_program (); 
  float scale0[3] = {dopts.scale, dopts.scale, dopts.scale};
  const glgrib_palette & p = dopts.palette;

  p.setRGBA255 (program->programID);


  glUniform3fv (glGetUniformLocation (program->programID, "scale0"), 1, scale0);
  glUniform1f (glGetUniformLocation (program->programID, "valmin"), valmin);
  glUniform1f (glGetUniformLocation (program->programID, "valmax"), valmax);

  float palmax = p.hasMax () ? p.getMax () : valmax;
  float palmin = p.hasMin () ? p.getMin () : valmin;

  glUniform1f (glGetUniformLocation (program->programID, "palmin"), palmin);
  glUniform1f (glGetUniformLocation (program->programID, "palmax"), palmax);

  glgrib_world::render (view);
}

glgrib_field::~glgrib_field ()
{
}


