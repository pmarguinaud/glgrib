#include "glGribView.h"
#include "glGribTrigonometry.h"
#include "glGribOpenGL.h"
#include "glGribProjection.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stdio.h>
#include <iostream>
#include <cctype>


void glGrib::View::delMVP (glGrib::Program * program) const
{
  glDisable (GL_SCISSOR_TEST);
}


void glGrib::View::setMVP (glGrib::Program * program) const
{
  program->set ("MVP", MVP);
  program->set ("proj", ps.current ()->getType ());


  program->set ("schmidt_apply", opts.zoom.on);

  if (opts.zoom.on)
    {
      float stretch = opts.zoom.stretch;
      float omc2 = 1.0f - 1.0f / (stretch * stretch);
      float opc2 = 1.0f + 1.0f / (stretch * stretch);
      program->set ("schmidt_rotd", zoom_rotd);
      program->set ("schmidt_roti", zoom_roti);
      program->set ("schmidt_omc2", omc2);
      program->set ("schmidt_opc2", opc2);
    }


  float lon0 = opts.lon + 180.0f;

  if (ps.current ()->setLon0 (lon0))
    program->set ("lon0", lon0);


  if (opts.clip.on)
    {
      float xmin = width  * opts.clip.xmin, xmax = width  * opts.clip.xmax, 
	    ymin = height * opts.clip.ymin, ymax = height * opts.clip.ymax;
      int type = ps.current ()->getType ();
      if ((type == glGrib::Projection::LATLON)
       || (type == glGrib::Projection::MERCATOR))
        {
          float xpos1, xpos2, ypos1, ypos2;
          float lon1 = lon0 + opts.clip.dlon, lon2 = lon0 - opts.clip.dlon, 
                lat1 = -90.0f + opts.clip.dlat, lat2 = +90.0f - opts.clip.dlat;

          getScreenCoordsFromLatLon (&xpos1, &ypos1, lat1, lon1);
          getScreenCoordsFromLatLon (&xpos2, &ypos2, lat2, lon2);

          xmin = std::max (xmin, xpos1); xmax = std::min (xmax, xpos2);
          ymin = std::max (ymin, ypos1); ymax = std::min (ymax, ypos2);
        }
      glEnable (GL_SCISSOR_TEST);
      glScissor (xmin, ymin, xmax-xmin, ymax-ymin);
    }
}

void glGrib::View::calcMVP () 
{
  glGrib::Projection::type pt = glGrib::Projection::typeFromString (opts.projection);
  ps.setType (pt);

  glm::vec3 pos
    (opts.distance * glm::cos (glm::radians (opts.lon)) * glm::cos (glm::radians (opts.lat)), 
     opts.distance * glm::sin (glm::radians (opts.lon)) * glm::cos (glm::radians (opts.lat)),
     opts.distance *                                      glm::sin (glm::radians (opts.lat)));

  viewport   = glm::vec4 (0.0f, 0.0f, static_cast<float> (width), static_cast<float> (height));

  float ratio = static_cast<float> (width) / static_cast<float> (height);

  glm::mat4 trans = glm::mat4 (1.0f);

  if ((! opts.center.on) && (ratio > 1.0f))
    trans = glm::translate (trans, glm::vec3 ((ratio - 1.0f) / 2.0f, 0.0f, 0.0f));
       
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

  projection = trans * p;

  view       = ps.current ()->getView (pos, opts.distance);
  model      = glm::mat4 (1.0f);
  MVP = projection * view * model; 

  float lon0 = opts.lon + 180.0f;
  ps.current ()->setLon0 (lon0);
}

void glGrib::View::setViewport (int w, int h)
{
  width = w;
  height = h;
  calcMVP ();
}

int glGrib::View::getScreenCoordsFromLatLon (float * xpos, float * ypos, float lat, float lon) const
{
  lat = glm::radians (lat); 
  lon = glm::radians (lon);

  glm::vec3 pos = lonlat2xyz (lon, lat); 

  return getScreenCoordsFromXYZ (xpos, ypos, pos);
}

int glGrib::View::getLatLonFromScreenCoords (float xpos, float ypos, float * lat, float * lon) const
{
  glm::vec3 pos;

  if (! getXYZFromScreenCoords (xpos, ypos, &pos))
    return 0;

  *lat = glm::degrees (glm::asin (pos.z));
  *lon = glm::degrees (glm::atan (pos.y, pos.x));

  return 1;
}

int glGrib::View::getScreenCoordsFromXYZ (float * xpos, float * ypos, const glm::vec3 & xyz) const
{
  glm::vec3 pos = ps.current ()->project (xyz);
  pos = project (pos);

  *xpos = pos.x;
  *ypos = pos.y;

  return 1;
}

int glGrib::View::getXYZFromScreenCoords (float xpos, float ypos, glm::vec3 * xyz) const
{
  glm::vec3 xa = unproject (glm::vec3 (xpos, ypos, +0.985601f));
  glm::vec3 xb = unproject (glm::vec3 (xpos, ypos, +0.900000f));

  if (! ps.current ()->unproject (xa, xb, xyz))
    return 0;

  return 1;
}

float glGrib::View::fracToDistAtNadir (float frac) const
{
  return pixelToDistAtNadir (height * frac);
}

float glGrib::View::pixelToDistAtNadir (float pixels) const
{
  float lon = deg2rad * opts.lon, lat = deg2rad * opts.lat;

  float coslon0 = cos (lon), sinlon0 = sin (lon);
  float coslat0 = cos (lat), sinlat0 = sin (lat);

  glm::vec3 pos0 = glm::vec3 (coslon0 * coslat0, sinlon0 * coslat0, sinlat0);

  float xpos0, ypos0;
  getScreenCoordsFromXYZ (&xpos0, &ypos0, pos0);
  
  float xpos1 = xpos0, ypos1 = ypos0 + pixels;

  glm::vec3 pos1;

  if (getXYZFromScreenCoords (xpos1, ypos1, &pos1))
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
      return pi;
    }
}

void glGrib::View::setup (const glGrib::OptionsView & o)
{
  opts = o;
  calcMVP ();
  calcZoom ();
}

glGrib::View::transform_type glGrib::View::typeFromString (std::string str)
{
  for (size_t i = 0; i < str.length (); i++)
    str[i] = std::toupper (str[i]);
#define if_type(x) if (str == #x) return x
    if_type (PERSPECTIVE);
    if_type (ORTHOGRAPHIC);
#undef if_type
  return PERSPECTIVE;
}

void glGrib::View::toggleTransformType () 
{ 
  if (opts.transformation == "PERSPECTIVE")
    opts.transformation = "ORTHOGRAPHIC";
  if (opts.transformation == "ORTHOGRAPHIC")
    opts.transformation = "PERSPECTIVE";
  calcMVP (); 
}


void glGrib::View::calcZoom ()
{
  float lonP = opts.zoom.lon, latP = opts.zoom.lat;

  glm::mat4 zoom4rotd, zoom4roti;
  zoom4rotd = glm::rotate (glm::mat4 (1.0f),
                           glm::radians (+90.0f-static_cast<float> (latP)), 
                           glm::vec3 (-sinf (glm::radians (lonP)),
                                      +cosf (glm::radians (lonP)),
                                      0.0f)) 
            * glm::rotate (glm::mat4 (1.0f),
                           glm::radians (+180.0f+static_cast<float> (lonP)),
                           glm::vec3 (0.0f, 0.0f, 1.0f));


  zoom4roti = glm::rotate (glm::mat4 (1.0f),
              glm::radians (-180.0f-static_cast<float> (lonP)),
              glm::vec3 (0.0f, 0.0f, 1.0f))
            * glm::rotate (glm::mat4 (1.0f),
                           glm::radians (-90.0f+static_cast<float> (latP)), 
                           glm::vec3 (-sinf (glm::radians (lonP)),
                                      +cosf (glm::radians (lonP)),
                                      0.0f));

  zoom_rotd = glm::mat3 (zoom4rotd);
  zoom_roti = glm::mat3 (zoom4roti);
}
