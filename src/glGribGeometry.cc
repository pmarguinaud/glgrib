#include "glGribGeometry.h"
#include "glGribLoader.h"
#include "glGribGeometryGaussian.h"
#include "glGribGeometryLatLon.h"
#include "glGribGeometryLambert.h"
#include "glGribLoader.h"

#include <iostream>
#include <openssl/md5.h>

#include <map>
#include <stdexcept>

namespace glGrib
{

namespace
{
class cache_t : public std::map <std::string,GeometryPtr> {};
cache_t cache;
}

void Geometry::clearCache ()
{
  cache.clear ();
}

GeometryPtr Geometry::load 
  (Loader * ld, const std::string & file, 
   const OptionsGeometry & opts, const int Nj)
{
  HandlePtr ghp;
  codes_handle * h = nullptr;
 
  if (file != "")
    {
      ghp = ld->handleFromFile (file);
      h = ghp == nullptr ? nullptr : ghp->getCodesHandle ();
    }

  long int gridDefinitionTemplateNumber = -1;

  if (h != nullptr)
    codes_get_long (h, "gridDefinitionTemplateNumber", &gridDefinitionTemplateNumber);

  GeometryPtr geom;
 
  // Read geometry metadata
  switch (gridDefinitionTemplateNumber)
    {
      case 30: case 33:
        geom = std::make_shared<GeometryLambert> (ghp);
        break;
      case 40: case 41: case 42: case 43:
        geom = std::make_shared<GeometryGaussian> (ghp);
	break;
      case 0:
        geom = std::make_shared<GeometryLatLon> (ghp);
	break;
      case -1:
        geom = std::make_shared<GeometryGaussian> (Nj);
	break;
      default:
        throw std::runtime_error (std::string ("Unexpected gridDefinitionTemplateNumber ") 
			        + std::to_string (gridDefinitionTemplateNumber));
    }


  auto it = cache.find (geom->md5 ());
  if (it != cache.end ())
    {
      GeometryPtr g = it->second;
      if (*g == *geom)  // Same geometry
        {
          geom = g;
	  goto found;
	}
    }

  geom->setup (ghp, opts);

  if (opts.check.on)
    geom->checkTriangles ();

  cache.insert (std::pair<std::string,GeometryPtr> (geom->md5 (), geom));

found:


  // Remove geometries with a single reference (they only belong to the cache)
again:
  for (it = cache.begin (); it != cache.end (); ++it)
    if (it->second.use_count () == 1)
      {
        cache.erase (it->second->md5 ());
        goto again;
      }

  return geom;
}

Geometry::~Geometry ()
{
  if (subgrid)
    delete subgrid;
}

const std::string Geometry::md5string (const unsigned char md5[]) const
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

void Geometry::checkTriangles () const
{
  int nt = grid.numberOfTriangles;
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

void Geometry::bindTriangles (int level) const
{
  const Geometry * geom = (level && subgrid) ? subgrid : this;
  geom->grid.elementbuffer->bind (GL_ELEMENT_ARRAY_BUFFER);
}

void Geometry::renderTriangles (int level) const
{
  const Geometry * geom = (level && subgrid) ? subgrid : this;
  if (geom->grid.ind_strip_size)
    {
      glEnable (GL_PRIMITIVE_RESTART);
      glPrimitiveRestartIndex (OpenGL::restart);
      glDrawElements (GL_TRIANGLE_STRIP, geom->grid.ind_strip_size, 
                      GL_UNSIGNED_INT, nullptr);
      glDisable (GL_PRIMITIVE_RESTART);
    }
  else
    {
      glDrawElements (GL_TRIANGLES, 3 * geom->grid.numberOfTriangles, 
                      GL_UNSIGNED_INT, nullptr);
    }
}

void Geometry::setProgramParameters (Program * program) const 
{
#include "shaders/include/geometry/types.h"
  
  (void)dumm_type;

  program->set ("geometry_type", geometry_none);
}

void Geometry::bindCoordinates (int attr) const
{
  if (crds.vertexbuffer != nullptr)
    {
      crds.vertexbuffer->bind (GL_ARRAY_BUFFER);
      if (attr >= 0)
        {
          glEnableVertexAttribArray (attr);
          glVertexAttribPointer (attr, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
        }
    }
  else
    {
      glDisableVertexAttribArray (attr);
      glVertexAttrib2f (attr, 0.0f, 0.0f);
    }
}

}
