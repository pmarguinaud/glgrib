#ifndef _GLGRIB_TRIGONOMETRY_H
#define _GLGRIB_TRIGONOMETRY_H

#include <math.h>

static const float rad2deg = 180.0f / M_PI;
static const float deg2rad = M_PI / 180.0f;
static const float twopi = 2.0f * M_PI;
static const float pi = M_PI;
static const float halfpi = M_PI / 2.0f;

#include <glm/glm.hpp>

static
glm::vec3 lonlat2xyz (const glm::vec2 & lonlat)
{
  float coslon = cos (lonlat.x), sinlon = sin (lonlat.x);
  float coslat = cos (lonlat.y), sinlat = sin (lonlat.y);
  return glm::vec3 (coslon * coslat, sinlon * coslat, sinlat);
}

static
glm::vec2 xyz2lonlat (const glm::vec3 & xyz)
{
  return glm::vec2 (atan2 (xyz.y, xyz.x), asin (xyz.z));
}


static
void lonlat2xyz (float lon, float lat, float * x, float * y, float * z)
{
  float coslon = cos (lon), sinlon = sin (lon);
  float coslat = cos (lat), sinlat = sin (lat);
  *x = coslon * coslat; 
  *y = sinlon * coslat; 
  *z = sinlat;
}

static
void xyz2lonlat (float x, float y, float z, float * lon, float * lat)
{
  *lon = atan2 (y, x);
  *lat = asin (z);
}


#endif
