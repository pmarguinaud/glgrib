#pragma once

#include <math.h>

static const float rad2deg = 180.0f / M_PI;
static const float deg2rad = M_PI / 180.0f;
static const float twopi = 2.0f * M_PI;
static const float pi = M_PI;
static const float halfpi = M_PI / 2.0f;

#include <glm/glm.hpp>

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

static
glm::vec3 lonlat2xyz (const glm::vec2 & lonlat)
{
  float x, y, z;
  lonlat2xyz (lonlat.x, lonlat.y, &x, &y, &z);
  return glm::vec3 (x, y, z);
}

static
glm::vec2 xyz2lonlat (const glm::vec3 & xyz)
{
  float lon, lat;
  xyz2lonlat (xyz.x, xyz.y, xyz.z, &lon, &lat);
  return glm::vec2 (lon, lat);
}

static
glm::vec3 lonlat2xyz (float lon, float lat)
{
  return lonlat2xyz (glm::vec2 (lon, lat));
}

static
glm::vec2 xyz2lonlat (float x, float y, float z)
{
  return xyz2lonlat (glm::vec3 (x, y, z));
}


