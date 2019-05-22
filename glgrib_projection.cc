#include "glgrib_projection.h"

#include <iostream>

static const float pi = 3.1415926;

glm::vec3 compNormedPos (const glm::vec3 & xyz)
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

glm::vec3 glgrib_projection_xyz::unproject (const glm::vec3 & coords) const
{
  return coords;
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

glm::vec3 glgrib_projection_latlon::unproject (const glm::vec3 & coords) const
{
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

glm::vec3 glgrib_projection_mercator::unproject (const glm::vec3 & coords) const
{
}

glm::vec3 glgrib_projection_polar_north::project (const glm::vec3 & xyz) const
{
  glm::vec3 normedPos = compNormedPos (xyz);
  return glm::vec3 (0.0f, normedPos.x / (+normedPos.z + 1.0f), 
                    normedPos.y / (+normedPos.z + 1.0f));
}

glm::vec3 glgrib_projection_polar_north::unproject (const glm::vec3 & coords) const
{
  float r2 = coords.y * coords.y + coords.z * coords.z;
  float z = (1.0f - r2) / (1.0f + r2);
  float x = (1.0f + z) * coords.y;
  float y = (1.0f + z) * coords.z;
  return glm::vec3 (x, y, z);
}

glm::vec3 glgrib_projection_polar_south::project (const glm::vec3 & xyz) const
{
  glm::vec3 normedPos = compNormedPos (xyz);
  return glm::vec3 (0.0f, normedPos.x / (-normedPos.z + 1.0f), 
                    normedPos.y / (-normedPos.z + 1.0f));
}

glm::vec3 glgrib_projection_polar_south::unproject (const glm::vec3 & coords) const
{
}

