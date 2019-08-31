#ifndef _GLGRIB_VIEW_H
#define _GLGRIB_VIEW_H

#include "glgrib_opengl.h"
#include "glgrib_program.h"
#include "glgrib_options.h"
#include "glgrib_projection.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>

class glgrib_view
{
public:
  enum transform_type
  {
    PERSPECTIVE=0,
    ORTHOGRAPHIC=1,
  };

  static transform_type typeFromString (std::string);

  glgrib_options_view opts;
  void setMVP (glgrib_program *) const;
  void calcMVP () const;
  void setViewport (int, int);
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

  int get_latlon_from_screen_coords (float, float, float *, float *) const;
  int get_screen_coords_from_latlon (float *, float *, float, float) const;
  int get_screen_coords_from_xyz (float *, float *, const glm::vec3 &) const;
  int get_xyz_from_screen_coords (float, float, glm::vec3 *) const;
  float pixel_to_dist_at_nadir (float) const;
  float frac_to_dist_at_nadir (float) const;

  void nextProjection () { ps.next (); opts.projection = ps.currentName (); calcMVP (); }
  glgrib_projection * getProjection () const { return ps.current (); }

  int getWidth () const { return width; }
  int getHeight () const { return height; }

  void toggleTransformType ();

  void init (const glgrib_options_view &);

  const glm::mat4 & getMVP () const { return MVP; }

private:
  int width, height;
  mutable glgrib_projection_set ps;
  mutable glm::mat4 Model, View, Projection, MVP;
  mutable glm::vec4 Viewport;
};

#endif
