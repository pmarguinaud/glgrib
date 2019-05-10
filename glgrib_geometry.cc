#include "glgrib_geometry.h"
#include "glgrib_geometry_gaussian.h"
#include <stdio.h>
#include <iostream>
#include <openssl/md5.h>
#include <map>

typedef std::map <std::string,glgrib_geometry_ptr*> cache_t;
static cache_t cache;


glgrib_geometry_ptr glgrib_geometry_load (const std::string & file, const glgrib_options & opts)
{
  FILE * in = NULL;
  int err = 0;
  in = fopen (file.c_str (), "r");
  codes_handle * h = codes_handle_new_from_file (0, in, PRODUCT_GRIB, &err);

  glgrib_geometry_ptr geom = std::make_shared<glgrib_geometry_gaussian> (h);
  geom->md5hash = geom->md5 ();

  auto it = cache.find (geom->md5hash);
  if (it != cache.end ())
    geom = *(it->second);
  else
    geom->init (opts, h);

  codes_handle_delete (h);
  fclose (in);

  return geom;
}

glgrib_geometry::~glgrib_geometry ()
{
  cache.erase (md5hash);
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


