#ifndef _GLGRIB_PROJECTION_H
#define _GLGRIB_PROJECTION_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


class glgrib_projection
{
public:
  virtual glm::vec3 project (const glm::vec3 &) const = 0;
};

class glgrib_projection_xyz : public glgrib_projection
{
public:
  virtual glm::vec3 project (const glm::vec3 &) const;
};

class glgrib_projection_latlon : public glgrib_projection
{
public:
  virtual glm::vec3 project (const glm::vec3 &) const;
  float lon0 = 180.0; // Latitude of right handside
};

class glgrib_projection_mercator : public glgrib_projection
{
public:
  virtual glm::vec3 project (const glm::vec3 &) const;
  float lon0 = 180.0; // Latitude of right handside
};

class glgrib_projection_polar_north : public glgrib_projection
{
public:
  virtual glm::vec3 project (const glm::vec3 &) const;
};

class glgrib_projection_polar_south : public glgrib_projection
{
public:
  virtual glm::vec3 project (const glm::vec3 &) const;
};

#endif
