#include "glgrib_view.h"
#include "glgrib_opengl.h"
#include "glgrib_projection.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stdio.h>
#include <iostream>
#include <cctype>


void glgrib_view::delMVP (glgrib_program * program) const
{
  glDisable (GL_SCISSOR_TEST);
}


void glgrib_view::setMVP (glgrib_program * program) const
{
  program->setMatrix4fv ("MVP", &MVP[0][0]);
  program->set1i ("proj", ps.current ()->getType ());
  float lon0 = opts.lon + 180.0f;
  if (ps.current ()->setLon0 (lon0))
    program->set1f ("lon0", lon0);


  if (opts.clip.on)
    {
      int type = ps.current ()->getType ();
      if ((type == glgrib_projection::LATLON)
       || (type == glgrib_projection::MERCATOR))
        {
          float xpos1, xpos2, ypos1, ypos2;
          float dlon = 10.0f, dlat = 5.0f;
          float lon1 = lon0 + opts.clip.dlon, lon2 = lon0 - opts.clip.dlon, 
                lat1 = -90.0f + opts.clip.dlat, lat2 = +90.0f - opts.clip.dlat;

          get_screen_coords_from_latlon (&xpos1, &ypos1, lat1, lon1);
          get_screen_coords_from_latlon (&xpos2, &ypos2, lat2, lon2);

          glEnable (GL_SCISSOR_TEST);
          glScissor (xpos1, ypos1, xpos2-xpos1, ypos2-ypos1);
        }
    }
}

void glgrib_view::calcMVP () const
{
  glgrib_projection::type pt = glgrib_projection::typeFromString (opts.projection);
  ps.setType (pt);

  glm::vec3 pos
    (opts.distance * glm::cos (glm::radians (opts.lon)) * glm::cos (glm::radians (opts.lat)), 
     opts.distance * glm::sin (glm::radians (opts.lon)) * glm::cos (glm::radians (opts.lat)),
     opts.distance *                                      glm::sin (glm::radians (opts.lat)));

  Viewport   = glm::vec4 (0.0f, 0.0f, (float)width, (float)height);

  float ratio = (float)width/(float)height;

  glm::mat4 Trans = glm::mat4 (1.0f);

  if ((! opts.center.on) && (ratio > 1.0f))
    Trans = glm::translate (Trans, glm::vec3 ((ratio - 1.0f) / 2.0f, 0.0f, 0.0f));
       
  glm::mat4 p;

  switch (typeFromString (opts.transformation))
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

  float lon0 = opts.lon + 180.0f;
  ps.current ()->setLon0 (lon0);
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

  glm::vec3 pos = glm::vec3 (coslon * coslat, sinlon * coslat, sinlat);

  return get_screen_coords_from_xyz (xpos, ypos, pos);
}

int glgrib_view::get_latlon_from_screen_coords (float xpos, float ypos, float * lat, float * lon) const
{
  glm::vec3 pos;

  if (! get_xyz_from_screen_coords (xpos, ypos, &pos))
    return 0;

  *lat = glm::degrees (glm::asin (pos.z));
  *lon = glm::degrees (glm::atan (pos.y, pos.x));

  return 1;
}

int glgrib_view::get_screen_coords_from_xyz (float * xpos, float * ypos, const glm::vec3 & xyz) const
{
  glm::vec3 pos = ps.current ()->project (xyz);
  pos = project (pos);

  *xpos = pos.x;
  *ypos = pos.y;

  return 1;
}

int glgrib_view::get_xyz_from_screen_coords (float xpos, float ypos, glm::vec3 * xyz) const
{
  glm::vec3 xa = unproject (glm::vec3 (xpos, ypos, +0.985601f));
  glm::vec3 xb = unproject (glm::vec3 (xpos, ypos, +0.900000f));

  if (! ps.current ()->unproject (xa, xb, xyz))
    return 0;

  return 1;
}

float glgrib_view::frac_to_dist_at_nadir (float frac) const
{
  return pixel_to_dist_at_nadir (height * frac);
}

float glgrib_view::pixel_to_dist_at_nadir (float pixels) const
{
  const double deg2rad = M_PI / 180.0;

  float lon = deg2rad * opts.lon, lat = deg2rad * opts.lat;

  float coslon0 = cos (lon), sinlon0 = sin (lon);
  float coslat0 = cos (lat), sinlat0 = sin (lat);

  glm::vec3 pos0 = glm::vec3 (coslon0 * coslat0, sinlon0 * coslat0, sinlat0);

  float xpos0, ypos0;
  get_screen_coords_from_xyz (&xpos0, &ypos0, pos0);
  
  float xpos1 = xpos0, ypos1 = ypos0 + pixels;

  glm::vec3 pos1;

  if (get_xyz_from_screen_coords (xpos1, ypos1, &pos1))
    {
      // Double precision required here
      glm::dvec3 dpos0 = pos0;
      glm::dvec3 dpos1 = pos1;
      dpos0 = glm::normalize (dpos0);
      dpos1 = glm::normalize (dpos1);
      return acos (glm::dot (dpos0, dpos1));
    }
  else
    {
std::cout << " M_PI" << std::endl;
      return M_PI;
    }
}

void glgrib_view::setup (const glgrib_options_view & o)
{
  opts = o;
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

void glgrib_view::toggleTransformType () 
{ 
  if (opts.transformation == "PERSPECTIVE")
    opts.transformation = "ORTHOGRAPHIC";
  if (opts.transformation == "ORTHOGRAPHIC")
    opts.transformation = "PERSPECTIVE";
  calcMVP (); 
}




