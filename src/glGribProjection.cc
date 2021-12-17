#include "glGribProjection.h"
#include "glGribTrigonometry.h"

#include <iostream>


namespace glGrib
{

namespace
{

const glm::vec3 intersectPlane 
  (const glm::vec3 & xa, const glm::vec3 & xb,
   const glm::vec3 & p, const glm::vec3 & v) 
{
// The plane is defined by the normal v and p which belongs to the plane
  glm::vec3 u = xb - xa;

  float lambda = glm::dot (v, p - xa) / glm::dot (u, v);

  return xa + lambda * u;
}

const glm::vec3 intersectSphere 
  (const glm::vec3 & xa, const glm::vec3 & xb,
   const glm::vec3 & c, const float & r) 
{
// The sphere is defined by the radius r and its centre c
  glm::vec3 u = xb - xa;
  glm::vec3 dx = xa - c;

  float A = glm::dot (u, u);
  float B = 2. * glm::dot (dx, u);
  float C = dot (dx, dx) - r * r;
  float Delta = B * B - 4. * A * C;

  if (Delta < 0)
    return c;

  float lambda = (-B + std::sqrt (Delta)) / (2. * A);

  return xa + lambda * u;
}

const glm::vec3 compNormedPos (const glm::vec3 & xyz)
{
  float x = xyz.x;
  float y = xyz.y;
  float z = xyz.z;
  float r = 1.0f / std::sqrt (x * x + y * y + z * z); 
  return glm::vec3 (x * r, y * r, z * r);
}

}

const glm::vec3 ProjectionXYZ::project 
(const glm::vec3 & xyz, const glm::mat3 &) const
{
  return xyz;
}

int ProjectionXYZ::unproject 
(const glm::vec3 & xa, const glm::vec3 & xb, 
 glm::vec3 * xyz, const glm::mat3 &) const
{
  glm::vec3 centre (0.0f, 0.0f, 0.0f);
  *xyz = intersectSphere (xa, xb, centre, 1.0f);
  if (centre == *xyz)
    return 0;
  return 1;
}

const glm::mat4 ProjectionXYZ::getView 
(const glm::vec3 & p, const float dist, const glm::mat3 &,
 const glm::vec3 & center, const glm::vec3 & up) const
{
  return glm::lookAt (p, center, up);
}

const glm::vec3 ProjectionLatLon::project 
(const glm::vec3 & xyz, const glm::mat3 & coordm) const
{
  float lat, lon;
  xyz2lonlat (compNormedPos (coordm * xyz), &lon, &lat);
  lon = glm::mod (lon, 2.0f * pi);
  float X = (glm::mod (lon - lon0 * pi / 180.0f, 2.0f * pi) - pi) / pi;
  float Y = lat / pi;
  return glm::vec3 (0.0f, X, Y);
}

int ProjectionLatLon::unproject 
(const glm::vec3 & xa, const glm::vec3 & xb, 
 glm::vec3 * xyz, const glm::mat3 & coordm) const
{
  glm::vec3 pos = intersectPlane (xa, xb,  glm::vec3 (0.0f, 0.0f, 0.0f), glm::vec3 (1.0f, 0.0f, 0.0f));
  float lon = pos.y * pi + lon0 * pi / 180.0f + pi;
  float lat = pos.z * pi;
  *xyz = lonlat2xyz (glm::vec2 (lon, lat));
  *xyz = glm::inverse (coordm) * (*xyz);
  return 1;
}

const glm::mat4 ProjectionLatLon::getView 
(const glm::vec3 & p, const float dist, 
 const glm::mat3 & coordm,
 const glm::vec3 & center, const glm::vec3 & up) const
{
  glm::vec3 co = project (p, coordm);
  return glm::lookAt (glm::vec3 (+dist, co.y, co.z), glm::vec3 (0.0f, +co.y, +co.z), glm::vec3 (0.0f, 0.0f, +1.0f));
}

const glm::vec3 ProjectionMercator::project 
(const glm::vec3 & xyz, const glm::mat3 & coordm) const
{
  float lon, lat;
  xyz2lonlat (compNormedPos (coordm * xyz), &lon, &lat);
  lon = glm::mod (lon, 2.0f * pi);
  float X = (glm::mod (lon - lon0 * pi / 180.0f, 2.0f * pi) - pi) / pi;
  float Y = glm::log (glm::tan (pi / 4.0f + lat / 2.0f)) / pi;
  return glm::vec3 (0.0f, X, Y);
}

int ProjectionMercator::unproject 
(const glm::vec3 & xa, const glm::vec3 & xb, 
 glm::vec3 * xyz, const glm::mat3 & coordm) const
{
  glm::vec3 pos = intersectPlane (xa, xb,  glm::vec3 (0.0f, 0.0f, 0.0f), glm::vec3 (1.0f, 0.0f, 0.0f));
  float lon = pos.y * pi + lon0 * pi / 180.0f + pi;
  float lat = 2.0f * glm::atan (glm::exp (pos.z * pi)) - pi / 2.0f;
  *xyz = lonlat2xyz (glm::vec2 (lon, lat));
  *xyz = glm::inverse (coordm) * (*xyz);
  return 1;
}

const glm::mat4 ProjectionMercator::getView 
(const glm::vec3 & p, const float dist, 
 const glm::mat3 & coordm,
 const glm::vec3 & center, const glm::vec3 & up) const
{
  glm::vec3 co = project (p, coordm);
  return glm::lookAt (glm::vec3 (+dist, co.y, co.z), glm::vec3 (0.0f, +co.y, +co.z), glm::vec3 (0.0f, 0.0f, +1.0f));
}

const glm::vec3 ProjectionPolarNorth::project 
(const glm::vec3 & xyz, const glm::mat3 & coordm) const
{
  glm::vec3 normedPos = compNormedPos (coordm * xyz);
  return glm::vec3 (0.0f, normedPos.x / (+normedPos.z + 1.0f), 
                    normedPos.y / (+normedPos.z + 1.0f));
}

int ProjectionPolarNorth::unproject 
(const glm::vec3 & xa, const glm::vec3 & xb, 
 glm::vec3 * xyz, const glm::mat3 & coordm) const
{
  glm::vec3 pos = intersectPlane (xa, xb,  glm::vec3 (0.0f, 0.0f, 0.0f), glm::vec3 (1.0f, 0.0f, 0.0f));
  float r2 = pos.y * pos.y + pos.z * pos.z;
  float z = (-r2 + 1.0f) / (+r2 + 1.0f);
  float x = (1.0f + z) * pos.y;
  float y = (1.0f + z) * pos.z;
  *xyz = glm::vec3 (x, y, z);
  *xyz = glm::inverse (coordm) * (*xyz);
  return 1;
}

const glm::mat4 ProjectionPolarNorth::getView 
(const glm::vec3 & p, const float dist, 
 const glm::mat3 & coordm,
 const glm::vec3 & center, const glm::vec3 & up) const
{
  glm::vec3 co = project (p, coordm);
  return glm::lookAt (glm::vec3 (+dist, co.y, co.z), glm::vec3 (0.0f, +co.y, +co.z), glm::vec3 (0.0f, -co.y, -co.z));
}

const glm::vec3 ProjectionPolarSouth::project 
(const glm::vec3 & xyz, const glm::mat3 & coordm) const
{
  glm::vec3 normedPos = compNormedPos (coordm * xyz);
  return glm::vec3 (0.0f, normedPos.x / (-normedPos.z + 1.0f), 
                    normedPos.y / (-normedPos.z + 1.0f));
}

int ProjectionPolarSouth::unproject 
(const glm::vec3 & xa, const glm::vec3 & xb, 
 glm::vec3 * xyz, const glm::mat3 & coordm) const
{
  glm::vec3 pos = intersectPlane (xa, xb,  glm::vec3 (0.0f, 0.0f, 0.0f), glm::vec3 (1.0f, 0.0f, 0.0f));
  float r2 = pos.y * pos.y + pos.z * pos.z;
  float z = (+r2 - 1.0f) / (+r2 + 1.0f);
  float x = (1.0f - z) * pos.y;
  float y = (1.0f - z) * pos.z;
  *xyz = glm::vec3 (x, y, z);
  *xyz = glm::inverse (coordm) * (*xyz);
  return 1;
}

const glm::mat4 ProjectionPolarSouth::getView 
(const glm::vec3 & p, const float dist, 
 const glm::mat3 & coordm,
 const glm::vec3 & center, const glm::vec3 & up) const
{
  glm::vec3 co = project (p, coordm);
  return glm::lookAt (glm::vec3 (-dist, co.y, co.z), glm::vec3 (0.0f, +co.y, +co.z), glm::vec3 (0.0f, +co.y, +co.z));
}


Projection::type Projection::typeFromString (std::string str)
{
  for (size_t i = 0; i < str.length (); i++)
    str[i] = std::toupper (str[i]);
#define if_type(x) if (str == #x) return x
  if_type (XYZ);
  if_type (POLAR_NORTH);
  if_type (POLAR_SOUTH);
  if_type (MERCATOR);
  if_type (LATLON);
#undef if_type
  return XYZ;
}

}
