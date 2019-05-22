#include "glgrib_projection.h"

#include <iostream>

static const float pi = 3.1415926;

static glm::vec3 intersect_plane (const glm::vec3 & xa, const glm::vec3 & xb,
                                  const glm::vec3 & p, const glm::vec3 & v) 
{
// The plane is defined by the normal v and p which belongs to the plane
  glm::vec3 u = xb - xa;

  float lambda = glm::dot (v, p - xa) / glm::dot (u, v);

  return xa + lambda * u;
}

static glm::vec3 intersect_sphere (const glm::vec3 & xa, const glm::vec3 & xb,
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

  float lambda = (-B + sqrt (Delta)) / (2. * A);

  return xa + lambda * u;
}


static glm::vec3 compNormedPos (const glm::vec3 & xyz)
{
  float x = xyz.x;
  float y = xyz.y;
  float z = xyz.z;
  float r = 1.0f / sqrt (x * x + y * y + z * z); 
  return glm::vec3 (x * r, y * r, z * r);
}

glm::vec3 glgrib_projection_xyz::project (const glm::vec3 & xyz) const
{
  return xyz;
}

int glgrib_projection_xyz::unproject (const glm::vec3 & xa, const glm::vec3 & xb, glm::vec3 * xyz) const
{
  glm::vec3 centre (0.0f, 0.0f, 0.0f);
  *xyz = intersect_sphere (xa, xb, centre, 1.0f);
  if (centre == *xyz)
    return 0;
  return 1;
}

glm::mat4 glgrib_projection_xyz::getView (const glm::vec3 & p, const float dist) const
{
  return glm::lookAt (p, glm::vec3 (0.0f, 0.0f, 0.0f), glm::vec3 (0.0f, 0.0f, 1.0f));
}

glm::vec3 glgrib_projection_latlon::project (const glm::vec3 & xyz) const
{
  glm::vec3 normedPos = compNormedPos (xyz);
  float lat = glm::asin (normedPos.z);
  float lon = glm::mod (glm::atan (normedPos.y, normedPos.x), 2.0f * pi);
  float X = (glm::mod (lon - lon0 * pi / 180.0f, 2.0f * pi) - pi) / pi;
  float Y = lat / pi;
  return glm::vec3 (0.0f, X, Y);
}

int glgrib_projection_latlon::unproject (const glm::vec3 & xa, const glm::vec3 & xb, glm::vec3 * xyz) const
{
  return 0;
}

glm::mat4 glgrib_projection_latlon::getView (const glm::vec3 & p, const float dist) const
{
  return glm::lookAt (p, glm::vec3 (0.0f, 0.0f, 0.0f), glm::vec3 (0.0f, 0.0f, 1.0f));
}

glm::vec3 glgrib_projection_mercator::project (const glm::vec3 & xyz) const
{
  glm::vec3 normedPos = compNormedPos (xyz);
  float lat = glm::asin (normedPos.z);
  float lon = glm::mod (glm::atan (normedPos.y, normedPos.x), 2.0f * pi);
  float X = (glm::mod (lon - lon0 * pi / 180.0f, 2.0f * pi) - pi) / pi;
  float Y = log (glm::tan (pi / 4.0f + lat / 2.0f)) / pi;
  return glm::vec3 (0.0f, X, Y);
}

int glgrib_projection_mercator::unproject (const glm::vec3 & xa, const glm::vec3 & xb, glm::vec3 * xyz) const
{
  return 0;
}

glm::mat4 glgrib_projection_mercator::getView (const glm::vec3 & p, const float dist) const
{
  return glm::lookAt (p, glm::vec3 (0.0f, 0.0f, 0.0f), glm::vec3 (0.0f, 0.0f, 1.0f));
}

glm::vec3 glgrib_projection_polar_north::project (const glm::vec3 & xyz) const
{
  glm::vec3 normedPos = compNormedPos (xyz);
  return glm::vec3 (0.0f, normedPos.x / (+normedPos.z + 1.0f), 
                    normedPos.y / (+normedPos.z + 1.0f));
}

int glgrib_projection_polar_north::unproject (const glm::vec3 & xa, const glm::vec3 & xb, glm::vec3 * xyz) const
{
  glm::vec3 pos = intersect_plane (xa, xb,  glm::vec3 (0.0f, 0.0f, 0.0f), glm::vec3 (1.0f, 0.0f, 0.0f));
  float r2 = pos.y * pos.y + pos.z * pos.z;
  float z = (-r2 + 1.0f) / (+r2 + 1.0f);
  float x = (1.0f + z) * pos.y;
  float y = (1.0f + z) * pos.z;
  *xyz = glm::vec3 (x, y, z);
  return 1;
}

glm::mat4 glgrib_projection_polar_north::getView (const glm::vec3 & p, const float dist) const
{
  glm::vec3 co = project (p);
  return glm::lookAt (glm::vec3 (+dist, co.y, co.z), glm::vec3 (0.0f, +co.y, +co.z), glm::vec3 (0.0f, -co.y, -co.z));
}

glm::vec3 glgrib_projection_polar_south::project (const glm::vec3 & xyz) const
{
  glm::vec3 normedPos = compNormedPos (xyz);
  return glm::vec3 (0.0f, normedPos.x / (-normedPos.z + 1.0f), 
                    normedPos.y / (-normedPos.z + 1.0f));
}

int glgrib_projection_polar_south::unproject (const glm::vec3 & xa, const glm::vec3 & xb, glm::vec3 * xyz) const
{
  glm::vec3 pos = intersect_plane (xa, xb,  glm::vec3 (0.0f, 0.0f, 0.0f), glm::vec3 (1.0f, 0.0f, 0.0f));
  float r2 = pos.y * pos.y + pos.z * pos.z;
  float z = (+r2 - 1.0f) / (+r2 + 1.0f);
  float x = (1.0f - z) * pos.y;
  float y = (1.0f - z) * pos.z;
  *xyz = glm::vec3 (x, y, z);
  return 1;
}

glm::mat4 glgrib_projection_polar_south::getView (const glm::vec3 & p, const float dist) const
{
  glm::vec3 co = project (p);
  return glm::lookAt (glm::vec3 (-dist, co.y, co.z), glm::vec3 (0.0f, +co.y, +co.z), glm::vec3 (0.0f, +co.y, +co.z));
}





