#include "glgrib_field.h"
#include "glgrib_load.h"
#include "glgrib_program.h"
#include "glgrib_coords_world.h"
#include "glgrib_palette.h"

#include <stdlib.h>
#include <stdio.h>

void glgrib_field::init (const glgrib_options & o, const glgrib_coords_world * coords, const glgrib_geometry & geom)
{
  unsigned char * col;
  opts = o;

  ncol = 1;

  float * val;
  glgrib_load (opts.field.c_str (), &val, 2);

  col = (unsigned char *)malloc (ncol * coords->np * sizeof (unsigned char));

  for (int i = 0; i < coords->np; i++)
    col[i] = (int)(255 * val[i]);

  def_from_vertexbuffer_col_elementbuffer (coords, col, geom);

  free (col);
  free (val);
}

void glgrib_field::render (const glgrib_view * view) const
{
  if (! hidden)
    {
      const glgrib_program * program = get_program (); 
      float scale0[3] = {opts.field_scale, opts.field_scale, opts.field_scale};

      glgrib_palette p_cold_hot
        (
             0,   0, 255, 255,
           255, 255, 255, 255,
           255,   0,   0, 255 
        );

      glgrib_palette p_cloud
        (
           255, 255, 255,   0,
           255, 255, 255, 255
        );

      p_cloud.setRGBA255 (program->programID);

      glUniform3fv (glGetUniformLocation (program->programID, "scale0"), 1, scale0);
      glgrib_world::render (view);
    }
}



