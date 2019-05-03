#include "glgrib_field.h"
#include "glgrib_load.h"
#include "glgrib_program.h"
#include "glgrib_palette.h"

#include <stdlib.h>
#include <stdio.h>

void glgrib_field::init (const std::string & field, const glgrib_options & o, const glgrib_geometry * geom)
{
  unsigned char * col;

  geometry = geom;

  ncol = 1;

  glgrib_load (field, &values, &valmin, &valmax, &valmis);

  col = (unsigned char *)malloc (ncol * geom->np * sizeof (unsigned char));

  for (int i = 0; i < geom->np; i++)
    if (values[i] == valmis)
      col[i] = 0;
    else
      col[i] = 1 + (int)(254 * (values[i] - valmin)/(valmax - valmin));

  def_from_vertexbuffer_col_elementbuffer (col, geom);

  free (col);
}

void glgrib_field::render (const glgrib_view * view, const glgrib_field_display_options & dopts) const
{
  const glgrib_program * program = get_program (); 
  float scale0[3] = {dopts.scale, dopts.scale, dopts.scale};

  dopts.palette.setRGBA255 (program->programID);

  glUniform3fv (glGetUniformLocation (program->programID, "scale0"), 1, scale0);
  glgrib_world::render (view);
}

glgrib_field::~glgrib_field ()
{
  if (values)
    free (values);
}


