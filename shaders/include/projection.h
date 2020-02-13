#include "projection/kind.h"

uniform int proj_vs = 0;
uniform bool isflat = true;
uniform float lon0vs = 180.0; // Longitude of right handside
const float pi = 3.1415926;

vec3 posFromLonLat (vec2 vertexLonLat)
{
  float lon = vertexLonLat.x, lat = vertexLonLat.y;
  if (abs (lat) > pi)
    return vec3 (0.0f, 0.0f, 0.0f);
  float coslon = cos (lon), sinlon = sin (lon);
  float coslat = cos (lat), sinlat = sin (lat);
  return vec3 (coslon * coslat, sinlon * coslat, sinlat);
}

vec3 compNormedPos (vec3 vertexPos)
{
  float x = vertexPos.x;
  float y = vertexPos.y;
  float z = vertexPos.z;
  float r = 1. / sqrt (x * x + y * y + z * z);
  return vec3 (x * r, y * r, z * r);
}

vec3 compProjedPos (vec3 vertexPos, vec3 normedPos)
{
  vec3 pos;
  switch (proj_vs)
    {
      case XYZ:
        if (isflat)
          pos = normedPos;
        else
          pos = vertexPos;
        break;
      case POLAR_NORTH:
        pos =  vec3 (0., normedPos.x / (+normedPos.z + 1.0),
                     normedPos.y / (+normedPos.z + 1.0));
        break;
      case POLAR_SOUTH:
        pos =  vec3 (0., normedPos.x / (-normedPos.z + 1.0),
                     normedPos.y / (-normedPos.z + 1.0));
        break;
      case MERCATOR:
        {
          float lat = asin (normedPos.z);
          float lon = mod (atan (normedPos.y, normedPos.x), 2 * pi);
          float X = (mod (lon - lon0vs * pi / 180.0, 2 * pi) - pi) / pi;
          float Y = log (tan (pi / 4. + lat / 2.)) / pi;
          pos = vec3 (0., X, Y);
        }
        break;
      case LATLON:
        {
          float lat = asin (normedPos.z);
          float lon = mod (atan (normedPos.y, normedPos.x), 2 * pi);
          float X = (mod (lon - lon0vs * pi / 180.0, 2 * pi) - pi) / pi;
          float Y = lat / pi;
          pos = vec3 (0., X, Y);
        }
        break;
    }

  return pos;
}

