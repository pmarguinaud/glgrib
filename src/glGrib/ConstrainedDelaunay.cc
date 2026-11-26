#include "glGrib/ConstrainedDelaunay.h"

#include "CDT/CDT.h"

#include <cmath>
#include <algorithm>
#include <exception>

#include <glm/glm.hpp>

namespace glGrib
{

namespace
{

typedef glm::dvec3 vec3;

// Stereographic projection seen from -c ; with e1 x e2 = c, triangles that are
// counter-clockwise in the plane are counter-clockwise seen from outside the sphere
class stereo_t
{
public:
  explicit stereo_t (const vec3 & c0)
  {
    c = glm::normalize (c0);
    vec3 a = std::fabs (c.z) < 0.9 ? vec3 (0.0, 0.0, 1.0) : vec3 (1.0, 0.0, 0.0);
    e1 = glm::normalize (glm::cross (a, c));
    e2 = glm::cross (c, e1);
  }
  // Returns false when p is (nearly) at the antipode of c
  bool fwd (const vec3 & p, CDT::V2d<double> * xy) const
  {
    double d = 1.0 + glm::dot (p, c);
    xy->x = 2.0 * glm::dot (p, e1) / d;
    xy->y = 2.0 * glm::dot (p, e2) / d;
    return d > 1e-9;
  }
  vec3 inv (double x, double y) const
  {
    double t = 4.0 / (4.0 + x * x + y * y);
    return glm::normalize (t * x * e1 + t * y * e2 + (2.0 * t - 1.0) * c);
  }
private:
  vec3 c, e1, e2;
};

}

bool ConstrainedDelaunay::processRing
  (const std::vector<float> & lonlat, int off, int len,
   std::vector<unsigned int> * ind, std::vector<float> * added)
{
  ind->clear ();
  added->clear ();

  if (len < 3)
    return true;

  // Coordinates on the sphere, and centre of the ring
  std::vector<vec3> xyz (len);
  vec3 c (0.0);
  for (int i = 0; i < len; i++)
    {
      double lon = lonlat[2*(off+i)+0], lat = lonlat[2*(off+i)+1];
      xyz[i] = vec3 (std::cos (lon) * std::cos (lat), std::sin (lon) * std::cos (lat), std::sin (lat));
      c += xyz[i];
    }
  if (glm::length (c) < 1e-9)
    return false;

  stereo_t proj (c);

  std::vector<CDT::V2d<double> > verts (len);
  for (int i = 0; i < len; i++)
    if (! proj.fwd (xyz[i], &verts[i]))
      return false;

  std::vector<CDT::Edge> edges;
  edges.reserve (len);
  for (int i = 0; i < len; i++)
    edges.push_back (CDT::Edge (i, (i + 1) % len));

  try
    {
      // Identical points (repeated points, pole points) must be merged for CDT
      CDT::DuplicatesInfo dup = CDT::RemoveDuplicatesAndRemapEdges (verts, edges);

      std::vector<CDT::Edge> edges1;
      edges1.reserve (edges.size ());
      for (size_t i = 0; i < edges.size (); i++)
        if (edges[i].v1 () != edges[i].v2 ())
          edges1.push_back (edges[i]);

      // Crossing edges (invalid rings) are split at their intersection
      CDT::Triangulation<double> cdt (CDT::VertexInsertionOrder::Auto,
                                      CDT::IntersectingConstraintEdges::TryResolve, 0.0);
      cdt.insertVertices (verts);
      cdt.insertEdges (edges1);
      cdt.eraseOuterTrianglesAndHoles ();  // even-odd rule

      // Unique vertex -> first ring vertex ; new vertices -> len, len+1, ...
      const int nu = verts.size ();
      const int nv = cdt.vertices.size ();
      std::vector<unsigned int> map (nv);
      for (int i = len - 1; i >= 0; i--)
        map[dup.mapping[i]] = i;
      for (int i = nu; i < nv; i++)
        {
          map[i] = len + (i - nu);
          vec3 p = proj.inv (cdt.vertices[i].x, cdt.vertices[i].y);
          added->push_back (std::atan2 (p.y, p.x));
          added->push_back (std::asin (std::max (-1.0, std::min (1.0, p.z))));
        }

      ind->reserve (3 * cdt.triangles.size ());
      for (size_t t = 0; t < cdt.triangles.size (); t++)
        for (int k = 0; k < 3; k++)
          ind->push_back (map[cdt.triangles[t].vertices[k]]);
    }
  catch (const std::exception & e)
    {
      ind->clear ();
      added->clear ();
      return false;
    }

  return true;
}

}
