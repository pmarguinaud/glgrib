#include "glgrib_view.h"
#include "glgrib_opengl.h"
#include "glgrib_projection.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stdio.h>
#include <iostream>
#include <cctype>


void glgrib_view::setMVP (glgrib_program * program) const
{
  program->setMatrix4fv ("MVP", &MVP[0][0]);
  program->set1i ("proj", ps.current ()->getType ());
  float lon0 = opts.lon + 180.0f;
  if (ps.current ()->setLon0 (lon0))
    program->set1f ("lon0", lon0);
}

void glgrib_view::calcMVP () const
{
  glm::vec3 pos
    (opts.distance * glm::cos (glm::radians (opts.lon)) * glm::cos (glm::radians (opts.lat)), 
     opts.distance * glm::sin (glm::radians (opts.lon)) * glm::cos (glm::radians (opts.lat)),
     opts.distance *                                      glm::sin (glm::radians (opts.lat)));

  Viewport   = glm::vec4 (0.0f, 0.0f, (float)width, (float)height);

  float ratio = (float)width/(float)height;

  glm::mat4 Trans = glm::mat4 (1.0f);

  if (ratio > 1.0f)
    Trans = glm::translate (Trans, glm::vec3 ((ratio - 1.0f) / 2.0f, 0.0f, 0.0f));
       
  glm::mat4 p;

  switch (transtype)
    {
      case PERSPECTIVE:
        p = glm::perspective (glm::radians (opts.fov), ratio, 0.1f, 100.0f);
	break;
      case ORTHOGRAPHIC:
        const float margin = 1.05;
	float zoom = opts.fov / 20.0f;
        p = glm::ortho (-ratio * margin * zoom, +ratio * margin * zoom, 
			 -1.0f * margin * zoom,  +1.0f * margin * zoom, 
			 0.100f, 100.0f);
	break;
    }

  Projection = Trans * p;

  View       = ps.current ()->getView (pos, opts.distance);
  Model      = glm::mat4 (1.0f);
  MVP = Projection * View * Model; 

}

void glgrib_view::setViewport (int w, int h)
{
  width = w;
  height = h;
}

int glgrib_view::get_screen_coords_from_latlon (float * xpos, float * ypos, float lat, float lon) const
{

  lat = glm::radians (lat); 
  lon = glm::radians (lon);

  float coslon = glm::cos (lon), sinlon = glm::sin (lon);
  float coslat = glm::cos (lat), sinlat = glm::sin (lat);

  glm::vec3 xyz = glm::vec3 (coslon * coslat, sinlon * coslat, sinlat);

  glm::vec3 pos = ps.current ()->project (xyz);
  pos = project (pos);

  *xpos = pos.x;
  *ypos = pos.y;

  return 1;
}

int glgrib_view::get_latlon_from_screen_coords (float xpos, float ypos, float * lat, float * lon) const
{
  glm::vec3 pos;

  glm::vec3 xa = unproject (glm::vec3 (xpos, ypos, +0.985601f));
  glm::vec3 xb = unproject (glm::vec3 (xpos, ypos, +0.900000f));

  if (! ps.current ()->unproject (xa, xb, &pos))
    return 0;

  *lat = glm::degrees (glm::asin (pos.z));
  *lon = glm::degrees (glm::atan (pos.y, pos.x));

  return 1;
}

void glgrib_view::init (const glgrib_options & o)
{
  opts = o.camera;
  glgrib_projection::type pt = glgrib_projection::typeFromString (o.scene.projection);
  ps.setType (pt);
  transtype = glgrib_view::typeFromString (o.scene.transformation);
}

glgrib_view::transform_type glgrib_view::typeFromString (std::string str)
{
  for (int i = 0; i < str.length (); i++)
    str[i] = std::toupper (str[i]);
#define if_type(x) if (str == #x) return x
    if_type (PERSPECTIVE);
    if_type (ORTHOGRAPHIC);
#undef if_type
  return PERSPECTIVE;
}




