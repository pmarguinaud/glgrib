#include "glgrib_geometry.h"
#include "glgrib_geometry_gaussian.h"
#include <stdio.h>
#include <iostream>

glgrib_geometry * glgrib_geometry_load (const glgrib_options & opts)
{
  FILE * in = NULL;
  int err = 0;
  in = fopen (opts.landscape.geometry.c_str (), "r");
  codes_handle * h = codes_handle_new_from_file (0, in, PRODUCT_GRIB, &err);

  glgrib_geometry_gaussian * geom = new glgrib_geometry_gaussian (opts, h);

  codes_handle_delete (h);
  fclose (in);

  return geom;
}

glgrib_geometry::~glgrib_geometry ()
{
  glDeleteBuffers (1, &vertexbuffer);
  glDeleteBuffers (1, &elementbuffer);
}



