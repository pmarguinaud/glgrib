
#include "geometry/buffer_index.h"
#include "geometry/types.h"
#include "geometry/gaussian.h"
#include "geometry/lambert.h"
#include "geometry/latlon.h"


uniform int geometry_type = geometry_none;

vec2 getVertexLonLat (int jglo)
{
  if (geometry_type == geometry_gaussian)
    return getGaussianVertexLonLat (jglo);
  else if (geometry_type == geometry_lambert)
    return getLambertVertexLonLat (jglo);
  else if (geometry_type == geometry_latlon)
    return getLatLonVertexLonLat (jglo);
  else 
    return vertexLonLat;
}
