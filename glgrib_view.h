#ifndef _GLGRIB_VIEW_H
#define _GLGRIB_VIEW_H

#include "glgrib_opengl.h"
#include "glgrib_program.h"
#include "glgrib_options.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class glgrib_view
{
public:
  glgrib_options_camera opts;
  void setMVP (GLuint) const;
  void calcMVP () const;
  void setViewport (int, int);
  int width, height;
  mutable glm::mat4 Model, View, Projection, MVP;
  mutable glm::vec4 Viewport;
  glm::vec3 project (const glm::vec3 & xyz) const
  {
    return glm::project (xyz, View * Model, Projection, Viewport);
  }
  glm::vec3 unproject (const glm::vec3 & xyz) const
  {
    return glm::unProject (xyz, View * Model, Projection, Viewport);
  }
  glm::vec3 intersect_plane (const double &, const double &, const glm::vec3 &, const glm::vec3 &) const;
  glm::vec3 intersect_sphere (const double &, const double &, const glm::vec3 &, const float &) const;
  typedef enum
  {
    XYZ=0,
    POLAR_NORTH=1,
    POLAR_SOUTH=2,
    MERCATOR=3,
    LATLON=4 ,
    LAST=5
  } proj_type;
  proj_type proj = XYZ;
  int get_latlon_from_screen_coords (float, float, float *, float *);
};

#endif
