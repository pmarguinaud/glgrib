#include "glGrib/Geometry.h"
#include "glGrib/Loader.h"
#include "glGrib/GeometryGaussian.h"
#include "glGrib/GeometryLatLon.h"
#include "glGrib/GeometryLambert.h"
#include "glGrib/Loader.h"

#include <iostream>
#include <cmath>
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

int Geometry::locateTriangle (int it, const glm::vec3 & p) const
{
  // A triangle is left across edge (a, b) when p and the opposite vertex c lie on different
  // sides of the plane through a, b and the centre of the Earth; the vertex order does not matter.
  // getTriangleNeighbours gives in itri[k] the neighbour across edge (k, k+1), -1 at the domain edge.
  // Computed in double precision; a point on an edge (within eps) belongs to the current triangle.
  // A triangle with an edge shorter than epslen (about 6 m) has no area: this happens next to a
  // pole, where a row of points collapses to one point, up to rounding.
  const double eps = 1e-12, epslen = 1e-6;
  const glm::dvec3 P (p);
  const int maxsteps = getNumberOfTriangles () + 1;
  int prev = -1;

  for (int step = 0; (it >= 0) && (step < maxsteps); step++)
    {
      int jglo[3], itri[3];
      glm::vec3 xyz[3];
      getTriangleNeighbours (it, jglo, itri, xyz);

      // A triangle with no area contains nothing : step to a neighbour
      bool degenerate = false;
      for (int k = 0; k < 3; k++)
        if (glm::length (glm::cross (glm::dvec3 (xyz[k]), glm::dvec3 (xyz[(k+1)%3]))) < epslen)
          degenerate = true;
      if (degenerate)
        {
          int next = -1;
          for (int k = 0; k < 3; k++)
            if ((itri[k] >= 0) && (itri[k] != prev))
              {
                next = itri[k];
                break;
              }
          prev = it;
          it = next;
          continue;
        }

      int next = it;
      double worst = 0.0;
      for (int k = 0; k < 3; k++)
        {
          const glm::dvec3 A (xyz[k]), B (xyz[(k+1)%3]), C (xyz[(k+2)%3]);
          const glm::dvec3 N = glm::cross (A, B);
          const double len = glm::length (N);
          const double sp = glm::dot (N, P) / len, sc = glm::dot (N, C) / len;
          // Signed distance of p to the plane of the edge, positive on the side of c
          const double d = sc > 0.0 ? sp : -sp;
          if ((d < -eps) && (d < worst))
            {
              worst = d;
              next = itri[k];
            }
        }

      if (next == it)
        return it;

      prev = it;
      it = next;
    }

  return -1;
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
