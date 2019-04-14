#include "glgrib_world.h"
#include "glgrib_load.h"

#include <stdlib.h>

void world_t::init (const char * file)
{
  unsigned int * ind;
  float * xyz;
  unsigned char * col;
  ncol = use_alpha ? 4 : 3;

  glgrib_load (file, &np, &xyz, &col, &nt, &ind, use_alpha);
  
  def_from_xyz_col_ind (xyz, col, ind);

  free (ind);
  free (xyz);
  free (col);
}


