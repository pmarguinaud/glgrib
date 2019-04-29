#include "glgrib_geometry.h"
#include "glgrib_geometry_gaussian.h"
#include <stdio.h>

glgrib_geometry * glgrib_geometry_load (const glgrib_options & opts)
{
  FILE * in = NULL;
  int err = 0;
  in = fopen (opts.geometry.c_str (), "r");
  codes_handle * h = codes_handle_new_from_file (0, in, PRODUCT_GRIB, &err);

  glgrib_geometry_gaussian * geom = new glgrib_geometry_gaussian (opts, h);

  codes_handle_delete (h);
  fclose (in);

  return geom;
}

void glgrib_geometry::take_xyz_ind (int * _np, unsigned int * _nt, float ** _xyz, unsigned int ** _ind)
{
  *_np = np;
  *_nt = nt;
  *_xyz = xyz;
  *_ind = ind;
  xyz = NULL;
  ind = NULL;
}

glgrib_geometry::~glgrib_geometry ()
{
  if (xyz)
    free (xyz);
  xyz = NULL;
  if (ind)
    free (ind);
}



