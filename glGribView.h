#pragma once

#include "glGribOpengl.h"
#include "glGribProgram.h"
#include "glGribOptions.h"
#include "glGribProjection.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>

class glGribView
{
public:
  enum transform_type
  {
    PERSPECTIVE=0,
    ORTHOGRAPHIC=1,
  };

  static transform_type typeFromString (std::string);

  void setMVP (glGribProgram *) const;
  void delMVP (glGribProgram *) const;
  void calcMVP ();
  void setViewport (int, int);
  glm::vec3 project (const glm::vec3 & xyz) const
  {
    return glm::project (xyz, View * Model, Projection, Viewport);
  }
  glm::vec3 unproject (const glm::vec3 & xyz) const
  {
    return glm::unProject (xyz, View * Model, Projection, Viewport);
  }
  glm::vec3 intersectPlane (const double &, const double &, const glm::vec3 &, const glm::vec3 &) const;
  glm::vec3 intersectSphere (const double &, const double &, const glm::vec3 &, const float &) const;

  int getLatlonFromScreenCoords (float, float, float *, float *) const;
  int getScreenCoordsFromLatlon (float *, float *, float, float) const;
  int getScreenCoordsFromXyz (float *, float *, const glm::vec3 &) const;
  int getXyzFromScreenCoords (float, float, glm::vec3 *) const;
  float pixel_to_dist_at_nadir (float) const;
  float fracToDistAtNadir (float) const;

  void nextProjection () { ps.next (); opts.projection = ps.currentName (); calcMVP (); }
  glGribProjection * getProjection () const { return ps.current (); }

  int getWidth () const { return width; }
  int getHeight () const { return height; }

  void toggleTransformType ();

  void setup (const glGribOptionsView &);

  const glm::mat4 & getMVP () const { return MVP; }

  const glGribOptionsView & getOptions () const { return opts; }
  void setOptions (const glGribOptionsView & o) { opts = o; calcMVP (); }
  float getRatio () const { return (float)width/(float)height; }
private:
  glGribOptionsView opts;
  int width, height;
  glGribProjectionSet ps;
  glm::mat4 Model, View, Projection, MVP;
  glm::vec4 Viewport;
};

