#include "glgrib_field.h"
#include "glgrib_load.h"
#include "glgrib_program.h"
#include "glgrib_coords_world.h"

#include <stdlib.h>
#include <stdio.h>

void glgrib_field::init (const char * field, const glgrib_coords_world * coords)
{
  unsigned int * ind;
  float * xyz;
  unsigned char * col;

#ifdef UNDEF
  ncol = use_alpha () ? 4 : 3;

  col = (unsigned char *)malloc (ncol * coords->np * sizeof (unsigned char));

  for (int i = 0; i < coords->np; i++)
    {
      col[ncol*i+0] = (int)((float)(255 * i) / (float)coords->np);
      col[ncol*i+1] = 0;
      col[ncol*i+2] = 255 - col[ncol*i+0];
      if (ncol == 4)
        col[ncol*i+3] = 255;
    }
#else
  ncol = 1;

  col = (unsigned char *)malloc (ncol * coords->np * sizeof (unsigned char));

  for (int i = 0; i < coords->np; i++)
    col[i] = (int)((float)(255 * i) / (float)coords->np);

#endif

  def_from_vertexbuffer_col_elementbuffer (coords, col);

  free (col);
}

void glgrib_field::render (const glgrib_view * view) const
{
  if (! hidden)
    {
      const glgrib_program * program = get_program (); 
      GLint scale0 = glGetUniformLocation (program->programID, "scale0");
      float scale[3] = {1.1, 1.1, 1.1};
      glUniform3fv (scale0, 1, scale);
      glgrib_world::render (view);
    }
}



