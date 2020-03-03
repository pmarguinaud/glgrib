#include "glGribGrid.h"
#include "glGribTrigonometry.h"
#include "glGribProgram.h"

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <vector>

glGribGrid & glGribGrid::operator= (const glGribGrid & other)
{
  clear ();
  if ((this != &other) && other.isReady ())
    {
      glGribObject::operator= (other);
      opts = other.opts;
      setup (opts);
      setReady ();
    }
  return *this;
}

void glGribGrid::setup (const glGribOptionsGrid & o)
{
  opts = o;

  glGenVertexArrays (1, &VertexArrayID);
  glBindVertexArray (VertexArrayID);


  if (opts.labels.on)
    {
      
      glGribFontPtr font = newGlgribFontPtr (opts.labels.font);
      labels.setShared (true);
      labels.setChange (false);

      std::vector<std::string> L;
      std::vector<float> X, Y, Z, A;

      X.reserve (opts.resolution); Y.reserve (opts.resolution);
      Z.reserve (opts.resolution); A.reserve (opts.resolution);
      L.reserve (opts.resolution);

      auto push = [&X, &Y, &Z, &A, &L, this] (float lon, float lat, const std::string & l)
      {
        float x, y, z; 
        float a = opts.labels.angle;
        lonlat2xyz (deg2rad * lon, deg2rad * lat, &x, &y, &z);
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

      labels.setup3D (font, L, X, Y, Z, A, opts.labels.font.scale, glGribString::C);
      labels.setForegroundColor (opts.labels.font.color.foreground);
      labels.setBackgroundColor (opts.labels.font.color.background);
    }


  setReady ();
}

void glGribGrid::clear ()
{
  if (isReady ()) 
    glDeleteVertexArrays (1, &VertexArrayID);
  glGribObject::clear (); 
  labels.clear ();
}

glGribGrid::~glGribGrid ()
{
  clear (); 
}

void glGribGrid::render (const glGribView & view, const glGribOptionsLight & light) const
{
  glGribProgram * program = glGribProgram::load (glGribProgram::GRID);
  program->use ();

  view.setMVP (program);
  program->set ("color0", opts.color);
  program->set ("do_alpha", 0);
  program->set ("scale", opts.scale);

  program->set ("resolution", opts.resolution);
  program->set ("nn", opts.points);
  program->set ("frag_resolution", opts.resolution);
  program->set ("frag_nn", opts.points);
  program->set ("interval", opts.interval);
  program->set ("dash_length", opts.dash_length);

  glBindVertexArray (VertexArrayID);

  program->set ("do_lat", 0);
  program->set ("frag_do_lat", 0);
  glDrawArrays (GL_LINE_STRIP, 0, (opts.resolution - 1) * (opts.points + 1));

  program->set ("do_lat", 1);
  program->set ("frag_do_lat", 1);
  glDrawArrays (GL_LINE_STRIP, 0, 2 * opts.resolution * (opts.points / 2 + 1));

  glBindVertexArray (0);

  view.delMVP (program);

  labels.render (view);

}

