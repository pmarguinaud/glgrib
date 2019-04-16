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
      float R0[256], G0[256], B0[256], A0[256];

      for (int i = 0; i < 256; i++)
        {
          R0[i] = (float)i/255.;
          G0[i] = 0;
          B0[i] = 1. - R0[i];
	  A0[i] = (i % 255)/(float)255;
	}

      glUniform3fv (glGetUniformLocation (program->programID, "scale0"), 1, scale0);
      glUniform1fv (glGetUniformLocation (program->programID, "R0"), 256, R0);
      glUniform1fv (glGetUniformLocation (program->programID, "G0"), 256, G0);
      glUniform1fv (glGetUniformLocation (program->programID, "B0"), 256, B0);
      glUniform1fv (glGetUniformLocation (program->programID, "A0"), 256, A0);

      glgrib_world::render (view);
    }
}



