#pragma once

#include <cmath>
#include <glm/glm.hpp>

namespace glGrib
{

static const float rad2deg = 180.0f / M_PI;
static const float deg2rad = M_PI / 180.0f;
static const float twopi = 2.0f * M_PI;
static const float pi = M_PI;
static const float halfpi = M_PI / 2.0f;


static inline
void lonlat2xyz (float lon, float lat, float * x, float * y, float * z)
{
  float coslon = std::cos (lon), sinlon = std::sin (lon);
  float coslat = std::cos (lat), sinlat = std::sin (lat);
  *x = coslon * coslat; 
  *y = sinlon * coslat; 
  *z = sinlat;
}

static inline
void xyz2lonlat (float x, float y, float z, float * lon, float * lat)
{
  *lon = atan2 (y, x);
  *lat = std::asin (z);
}

static inline
glm::vec3 lonlat2xyz (const glm::vec2 & lonlat)
{
  float x, y, z;
  lonlat2xyz (lonlat.x, lonlat.y, &x, &y, &z);
  return glm::vec3 (x, y, z);
}

static inline
glm::vec2 xyz2lonlat (const glm::vec3 & xyz)
{
  float lon, lat;
  xyz2lonlat (xyz.x, xyz.y, xyz.z, &lon, &lat);
  return glm::vec2 (lon, lat);
}

static inline
glm::vec3 lonlat2xyz (float lon, float lat)
{
  return lonlat2xyz (glm::vec2 (lon, lat));
}

static inline
glm::vec2 xyz2lonlat (float x, float y, float z)
{
  return xyz2lonlat (glm::vec3 (x, y, z));
}

static inline
void xyz2lonlat (const glm::vec3 & xyz, float * lon, float * lat)
{
  xyz2lonlat (xyz.x, xyz.y, xyz.z, lon, lat);
}

static inline
void lonlat2xyz (const glm::vec2 & lonlat, float * x, float * y, float * z)
{
  lonlat2xyz (lonlat.x, lonlat.y, x, y, z);
}


}
