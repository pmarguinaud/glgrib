#include "glgrib_geometry.h"
#include "glgrib_geometry_gaussian.h"
#include "glgrib_geometry_latlon.h"

#include <stdio.h>
#include <iostream>
#include <openssl/md5.h>

#include <map>
#include <stdexcept>

typedef std::map <std::string,glgrib_geometry_ptr> cache_t;
static cache_t cache;


glgrib_geometry_ptr glgrib_geometry_load (const std::string & file, 
		                          const glgrib_options * opts)
{
  FILE * in = NULL;
  int err = 0;
  in = fopen (file.c_str (), "r");
  codes_handle * h = codes_handle_new_from_file (0, in, PRODUCT_GRIB, &err);

  long int gridDefinitionTemplateNumber;
  codes_get_long (h, "gridDefinitionTemplateNumber", &gridDefinitionTemplateNumber);

  glgrib_geometry_ptr geom;
 
  switch (gridDefinitionTemplateNumber)
    {
      case 43:
        geom = std::make_shared<glgrib_geometry_gaussian> (h);
	break;
      case 0:
        geom = std::make_shared<glgrib_geometry_latlon> (h);
	break;
      default:
        throw std::runtime_error (std::string ("Unexpected gridDefinitionTemplateNumber ") 
			        + std::to_string (gridDefinitionTemplateNumber));
    }

  auto it = cache.find (geom->md5 ());
  if (it != cache.end ())
    {
      geom = it->second;
    }
  else
    {
      geom->init (h, opts);
      cache.insert (std::pair<std::string,glgrib_geometry_ptr>
		    (geom->md5 (), geom));
    }

  codes_handle_delete (h);
  fclose (in);

  // Remove geometries with a single reference (they only belong to the cache)
again:
  for (it = cache.begin (); it != cache.end (); it++)
    if (it->second.use_count () == 1)
      {
        cache.erase (it->second->md5 ());
        goto again;
      }

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


