#include "glgrib_geometry.h"
#include "glgrib_geometry_gaussian.h"
#include <stdio.h>
#include <iostream>
#include <openssl/md5.h>

glgrib_geometry_ptr glgrib_geometry_load (const glgrib_options & opts)
{
  FILE * in = NULL;
  int err = 0;
  in = fopen (opts.landscape.geometry.c_str (), "r");
  codes_handle * h = codes_handle_new_from_file (0, in, PRODUCT_GRIB, &err);

  glgrib_geometry_ptr geom = std::make_shared<glgrib_geometry_gaussian> (opts, h);

  codes_handle_delete (h);
  fclose (in);

  return geom;
}

glgrib_geometry::~glgrib_geometry ()
{
}

std::string glgrib_geometry::md5string (const unsigned char md5[]) const
{
  const char * const lut = "0123456789ABCDEF";

  std::string str;
  str.reserve (2 * MD5_DIGEST_LENGTH);

  for (size_t i = 0; i < MD5_DIGEST_LENGTH; i++)
    {
      const unsigned char c = md5[i];
      str.push_back (lut[c >> 4]);
      str.push_back (lut[c & 15]);
    }

  return str;
}


