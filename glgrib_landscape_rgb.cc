#include "glgrib_landscape_rgb.h"
#include "glgrib_load.h"
#include "glgrib_program.h"
#include "glgrib_coords_world.h"

#include <stdlib.h>
#include <stdio.h>

void glgrib_landscape_rgb::init (const char * geom, const glgrib_coords_world * coords)
{
  unsigned char * col;

  ncol = use_alpha () ? 4 : 3;

  glgrib_load_rgb (geom, &col, use_alpha ());
  def_from_vertexbuffer_col_elementbuffer (coords, col);

  free (col);
}


