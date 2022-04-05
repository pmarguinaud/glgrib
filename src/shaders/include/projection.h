#include "projection/kind.h"

uniform int proj = 0;
uniform bool isflat = true;
uniform float lon0 = 180.0; // Longitude of right handside
const float pi = 3.1415926;

uniform bool apply_coordm = false;
uniform mat3 COORDM;

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
  normedPos = applySchmidt (normedPos);
  switch (proj)
    {
      case XYZ:
        if (isflat)
          pos = applySchmidt (normedPos);
        else
          pos = applySchmidt (vertexPos);
        break;
      case POLAR_NORTH:
        {
          vec3 p = apply_coordm ? COORDM * normedPos : normedPos;
          pos =  vec3 (0., p.x / (+p.z + 1.0),
                       p.y / (+p.z + 1.0));
        }
        break;
      case POLAR_SOUTH:
        {
          vec3 p = apply_coordm ? COORDM * normedPos : normedPos;
          pos =  vec3 (0., p.x / (-p.z + 1.0),
                       p.y / (-p.z + 1.0));
        }
        break;
      case MERCATOR:
        {
          vec3 p = apply_coordm ? COORDM * normedPos : normedPos;
          float lat = asin (p.z);
          float lon = mod (atan (p.y, p.x), 2 * pi);
          float X = (mod (lon - lon0 * pi / 180.0, 2 * pi) - pi) / pi;
          float Y = log (tan (pi / 4. + lat / 2.)) / pi;
          pos = vec3 (0., X, Y);
        }
        break;
      case LATLON:
        {
          vec3 p = apply_coordm ? COORDM * normedPos : normedPos;
          float lat = asin (p.z);
          float lon = mod (atan (p.y, p.x), 2 * pi);
          float X = (mod (lon - lon0 * pi / 180.0, 2 * pi) - pi) / pi;
          float Y = lat / pi;
          pos = vec3 (0., X, Y);
        }
        break;
    }

  return pos;
}

