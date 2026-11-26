#pragma once

#include <vector>

namespace glGrib
{

namespace ConstrainedDelaunay
{

// Triangulate a ring on the sphere with a constrained Delaunay triangulation (CDT library),
// in a stereographic projection centred on the ring.
//
// lonlat : coordinates (radians), the ring is lonlat[2*off .. 2*(off+len)), not closed
// ind    : output triangles, with local indices : 0 .. len-1 for the ring vertices,
//          len, len+1, ... for the points added by CDT (if edges cross)
// added  : output, coordinates (lon, lat radians) of the added points
//
// Returns false if CDT fails ; ind and added are then left empty.

bool processRing (const std::vector<float> & lonlat, int off, int len,
                  std::vector<unsigned int> * ind, std::vector<float> * added);

}

}
