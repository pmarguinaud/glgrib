#include "glgrib_world.h"
#include "glgrib_load.h"
#include "glgrib_program.h"

#include <stdlib.h>
#include <stdio.h>

void glgrib_world::init (const char * geom)
{
  unsigned int * ind;
  float * xyz;
  unsigned char * col;

  ncol = use_alpha () ? 4 : 3;

  glgrib_load (geom, &np, &xyz, &col, &nt, &ind, use_alpha ());
  def_from_xyz_col_ind (xyz, col, ind);

  free (ind);
  free (xyz);
  free (col);
}


