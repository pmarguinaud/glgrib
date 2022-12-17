#pragma once

#include "glGrib/OpenGL.h"
#include "glGrib/Program.h"
#include "glGrib/Options.h"
#include "glGrib/Projection.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>

namespace glGrib
{

class View
{
public:
  enum transform_type
  {
    PERSPECTIVE=0,
    ORTHOGRAPHIC=1,
  };

  static transform_type typeFromString (std::string);

  void setMVP (Program *) const;
  void delMVP (Program *) const;
  void calcMVP ();
  void calcCoordm ();
  void setViewport (int, int);
  const glm::vec3 project (const glm::vec3 & xyz) const
  {
    return glm::project (xyz, view * model, projection, viewport);
  }
  const glm::vec3 unproject (const glm::vec3 & xyz) const
  {
    return glm::unProject (xyz, view * model, projection, viewport);
  }
  const glm::vec3 intersectPlane (const double &, const double &, const glm::vec3 &, const glm::vec3 &) const;
  const glm::vec3 intersectSphere (const double &, const double &, const glm::vec3 &, const float &) const;

  int getLatLonFromScreenCoords (float, float, float *, float *) const;
  int getScreenCoordsFromLatLon (float *, float *, float, float) const;
  int getScreenCoordsFromXYZ (float *, float *, const glm::vec3 &) const;
  int getXYZFromScreenCoords (float, float, glm::vec3 *) const;
  float pixelToDistAtNadir (float) const;
  float fracToDistAtNadir (float) const;

  void nextProjection () { ps.next (); opts.projection = ps.currentName (); calcMVP (); }
  Projection * getProjection () const { return ps.current (); }

  int getWidth () const { return width; }
  int getHeight () const { return height; }

  void toggleTransformType ();

  void setup (const OptionsView &);

  const glm::mat4 & getMVP () const { return MVP; }

  const OptionsView & getOptions () const { return opts; }
  void setOptions (const OptionsView & o) { opts = o; calcMVP (); calcZoom (); }
  float getRatio () const { return (float)width/(float)height; }
  void calcZoom ();
private:
  const glm::mat3 & getCoordm () const;
  OptionsView opts;
  int width, height;
  ProjectionSet ps;
  glm::mat4 model, view, projection, MVP;
  glm::mat3 zoom_roti, zoom_rotd;
  glm::vec4 viewport;
  glm::mat3 coordm;
};


}
