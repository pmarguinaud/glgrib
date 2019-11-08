#include "glgrib_geometry.h"
#include "glgrib_loader.h"
#include "glgrib_geometry_gaussian.h"
#include "glgrib_geometry_latlon.h"
#include "glgrib_geometry_lambert.h"
#include "glgrib_loader.h"

#include <stdio.h>
#include <iostream>
#include <openssl/md5.h>

#include <map>
#include <stdexcept>

typedef std::map <std::string,glgrib_geometry_ptr> cache_t;
static cache_t cache;

glgrib_geometry_ptr glgrib_geometry_load (glgrib_loader * ld, const std::string & file, const float orography, const int Nj)
{
  glgrib_handle_ptr ghp;
  codes_handle * h = NULL;
 
  if (file != "")
    {
      ghp = ld->handle_from_file (file);
      h = ghp->getCodesHandle ();
    }

  long int gridDefinitionTemplateNumber = -1;

  if (h != NULL)
    codes_get_long (h, "gridDefinitionTemplateNumber", &gridDefinitionTemplateNumber);

  glgrib_geometry_ptr geom;
 
  // Read geometry metadata
  switch (gridDefinitionTemplateNumber)
    {
      case 30: case 33:
        geom = std::make_shared<glgrib_geometry_lambert> (ghp);
        break;
      case 40: case 41: case 42: case 43:
        geom = std::make_shared<glgrib_geometry_gaussian> (ghp);
	break;
      case 0:
        geom = std::make_shared<glgrib_geometry_latlon> (ghp);
	break;
      case -1:
        geom = std::make_shared<glgrib_geometry_gaussian> (Nj);
	break;
      default:
        throw std::runtime_error (std::string ("Unexpected gridDefinitionTemplateNumber ") 
			        + std::to_string (gridDefinitionTemplateNumber));
    }


  auto it = cache.find (geom->md5 ());
  if (it != cache.end ())
    {
      glgrib_geometry_ptr g = it->second;
      if (*g == *geom)  // Same geometry
        {
          geom = g;
	  goto found;
	}
    }

  geom->setup (ghp, orography);
  cache.insert (std::pair<std::string,glgrib_geometry_ptr> (geom->md5 (), geom));

found:


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

void glgrib_geometry::checkTriangles () const
{
  int nt = numberOfTriangles;
#pragma omp parallel for
  for (int it0 = 0; it0 < nt; it0++)
    {
      int itri0[3], jglo0[3]; glm::vec3 xyz0[3];
      getTriangleNeighbours (it0, jglo0, itri0, xyz0);
      for (int i0 = 0; i0 < 3; i0++)
        {
          int j0 = (i0 + 1) % 3;
          int it1 = itri0[i0];
	  if (it1 >= 0)
	    {
              int itri1[3], jglo1[3]; glm::vec3 xyz1[3];
              getTriangleNeighbours (it1, jglo1, itri1, xyz1);
	      bool found = false, adj = false;
              for (int i1 = 0; i1 < 3; i1++)
                {
                  int j1 = (i1 + 1) % 3;
                  if (itri1[i1] == it0)
                    {
                      found = true;
                      adj = ((jglo0[i0] == jglo1[i1]) && (jglo0[j0] == jglo1[j1]))
                         || ((jglo0[i0] == jglo1[j1]) && (jglo0[j0] == jglo1[i1]));
	              break;
	            }
                }
	      if (! found)
                printf ("Triangle %d is the neighbour of triangle %d, but the opposite is false\n", it0, it1);
	      else if (! adj)
	        printf ("Triangles %d and %d are not neighbours along the same edge\n", it0, it1);
	    }
	}
    }


  printf ("checkTriangles OK\n");
}




