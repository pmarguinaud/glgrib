#include "glgrib_field.h"
#include "glgrib_load.h"
#include "glgrib_program.h"
#include "glgrib_coords_world.h"
#include "glgrib_palette.h"

#include <stdlib.h>
#include <stdio.h>

void glgrib_field::init (const char * field, const glgrib_coords_world * coords)
{
  unsigned int * ind;
  float * xyz;
  unsigned char * col;

  ncol = 1;

  col = (unsigned char *)malloc (ncol * coords->np * sizeof (unsigned char));

  for (int i = 0; i < coords->np; i++)
    col[i] = (int)((float)(255 * i) / (float)coords->np);

  def_from_vertexbuffer_col_elementbuffer (coords, col);

  free (col);
}

void glgrib_field::render (const glgrib_view * view) const
{
  if (! hidden)
    {
      const glgrib_program * program = get_program (); 
      float scale0[3] = {1.1, 1.1, 1.1};

      glgrib_palette p (  0,   0, 255,   0,
		        255,   0,   0, 255);

      p.setRGBA255 (program->programID);

      glUniform3fv (glGetUniformLocation (program->programID, "scale0"), 1, scale0);
      glgrib_world::render (view);
    }
}



