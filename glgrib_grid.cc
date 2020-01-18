#include "glgrib_grid.h"
#include "glgrib_trigonometry.h"
#include "glgrib_program.h"

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <vector>

glgrib_grid & glgrib_grid::operator= (const glgrib_grid & other)
{
  clear ();
  if ((this != &other) && other.isReady ())
    {
      glgrib_object::operator= (other);
      opts = other.opts;
      setup (opts);
      setReady ();
    }
  return *this;
}

void glgrib_grid::setup (const glgrib_options_grid & o)
{
  opts = o;

  glGenVertexArrays (1, &VertexArrayID);
  glBindVertexArray (VertexArrayID);


  if (opts.labels.on)
    {
      
      glgrib_font_ptr font = new_glgrib_font_ptr (opts.labels.font);
      labels.setShared (true);
      labels.setChange (false);

      std::vector<std::string> L;
      std::vector<float> X, Y, Z, A;

      X.reserve (opts.resolution); Y.reserve (opts.resolution);
      Z.reserve (opts.resolution); A.reserve (opts.resolution);
      L.reserve (opts.resolution);

      auto push = [&X, &Y, &Z, &A, &L, this] (float lon, float lat, const std::string & l)
      {
	float coslon = cos (deg2rad * lon), sinlon = sin (deg2rad * lon);
	float coslat = cos (deg2rad * lat), sinlat = sin (deg2rad * lat);
        float x = coslon * coslat, y = sinlon * coslat, z = sinlat;
        float a = opts.labels.angle;
        X.push_back (x); Y.push_back (y);
	Z.push_back (z); A.push_back (a);
	L.push_back (l);
      };

      for (int i = 1; i < opts.resolution-1; i++)
        {
          float lon = opts.labels.lon, lat = 180.0f / 2 - i * 180.0f / opts.resolution;
          char s[64];
          sprintf (s, "%+4.0f", lat);
          std::string str = std::string (s);
	  push (lon, lat, str);
	}

      for (int i = -opts.resolution; i < opts.resolution; i++)
        {
          float lon = i * 360.0f / (2.0f * opts.resolution), lat = opts.labels.lat;
          char s[64];
          sprintf (s, "%+4.0f", lon);
          std::string str = std::string (s);
	  push (lon, lat, str);
	}

      labels.setup3D (font, L, X, Y, Z, A, opts.labels.font.scale, glgrib_string::C);
      labels.setForegroundColor (opts.labels.font.color.foreground);
    }


  setReady ();
}

void glgrib_grid::clear ()
{
  if (isReady ()) 
    glDeleteVertexArrays (1, &VertexArrayID);
  glgrib_object::clear (); 
  labels.clear ();
}

glgrib_grid::~glgrib_grid ()
{
  clear (); 
}

void glgrib_grid::render (const glgrib_view & view, const glgrib_options_light & light) const
{
  glgrib_program * program = glgrib_program::load (glgrib_program::GRID);
  program->use ();
  float color[3] = {(float)opts.color.r / 255.0f, 
                    (float)opts.color.g / 255.0f, 
                    (float)opts.color.b / 255.0f};

  view.setMVP (program);
  program->set3fv ("color0", color);
  program->set1i ("do_alpha", 0);
  program->set1f ("scale", opts.scale);

  program->set1i ("resolution", opts.resolution);
  program->set1i ("nn", opts.points);
  program->set1i ("frag_resolution", opts.resolution);
  program->set1i ("frag_nn", opts.points);
  program->set1i ("interval", opts.interval);
  program->set1f ("dash_length", opts.dash_length);

  glBindVertexArray (VertexArrayID);

  program->set1i ("do_lat", 0);
  program->set1i ("frag_do_lat", 0);
  glDrawArrays (GL_LINE_STRIP, 0, (opts.resolution - 1) * (opts.points + 1));

  program->set1i ("do_lat", 1);
  program->set1i ("frag_do_lat", 1);
  glDrawArrays (GL_LINE_STRIP, 0, 2 * opts.resolution * (opts.points / 2 + 1));

  glBindVertexArray (0);

  view.delMVP (program);

  labels.render (view);

}

