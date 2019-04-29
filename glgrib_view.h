#ifndef _GLGRIB_VIEW_H
#define _GLGRIB_VIEW_H

#include "glgrib_opengl.h"
#include "glgrib_program.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class glgrib_view
{
public:
  float rc = 6.0, latc = 0., lonc = 0., fov = 20.;
  void setMVP (GLuint) const;
  void calcMVP () const;
  void setViewport (int, int);
  int width, height;
  mutable glm::mat4 Model, View, Projection, MVP;
  glm::vec4 Viewport;
  glm::vec3 project (const glm::vec3 & xyz) const
  {
    return glm::project (xyz, View * Model, Projection, Viewport);
  }
  glm::vec3 unproject (const glm::vec3 & xyz) const
  {
    return glm::unProject (xyz, View * Model, Projection, Viewport);
  }
  glm::vec3 insersect_plane (const double &, const double &, const glm::vec3 &, const glm::vec3 &) const;
  glm::vec3 insersect_sphere (const double &, const double &, const glm::vec3 &, const float &) const;
};

#endif
