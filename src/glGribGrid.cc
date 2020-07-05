#include "glGribGrid.h"
#include "glGribTrigonometry.h"
#include "glGribProgram.h"

#include <cmath>
#include <iostream>
#include <vector>

namespace glGrib
{

void Grid::setup (const OptionsGrid & o)
{
  if (! o.on)
    return;

  d.opts = o;

  if (d.opts.labels.on)
    {
      FontPtr font = getGlGribFontPtr (d.opts.labels.font);

      std::vector<std::string> L;
      std::vector<float> X, Y, Z, A;

      X.reserve (d.opts.resolution); Y.reserve (d.opts.resolution);
      Z.reserve (d.opts.resolution); A.reserve (d.opts.resolution);
      L.reserve (d.opts.resolution);

      auto push = [&X, &Y, &Z, &A, &L, this] (float lon, float lat, const std::string & l)
      {
        float x, y, z; 
        float a = d.opts.labels.angle;
        lonlat2xyz (deg2rad * lon, deg2rad * lat, &x, &y, &z);
        X.push_back (x); Y.push_back (y);
	Z.push_back (z); A.push_back (a);
	L.push_back (l);
      };

      for (int i = 1; i < d.opts.resolution-1; i++)
        {
          float lon = d.opts.labels.lon, lat = 180.0f / 2 - i * 180.0f / d.opts.resolution;
          char s[64];
          sprintf (s, "%+4.0f", lat);
          std::string str = std::string (s);
	  push (lon, lat, str);
	}

      for (int i = -d.opts.resolution; i < d.opts.resolution; i++)
        {
          float lon = i * 360.0f / (2.0f * d.opts.resolution), lat = d.opts.labels.lat;
          char s[64];
          sprintf (s, "%+4.0f", lon);
          std::string str = std::string (s);
	  push (lon, lat, str);
	}

      d.labels.setup (font, L, X, Y, Z, A, d.opts.labels.font.scale, StringTypes::C);
      d.labels.setForegroundColor (d.opts.labels.font.color.foreground);
      d.labels.setBackgroundColor (d.opts.labels.font.color.background);
    }

  setReady ();
}

void Grid::render (const View & view, const OptionsLight & light) const
{
  Program * program = Program::load ("GRID");
  program->use ();

  view.setMVP (program);
  program->set ("color0", d.opts.color);
  program->set ("do_alpha", 0);
  program->set ("scale", d.opts.scale);

  program->set ("resolution", d.opts.resolution);
  program->set ("nn", d.opts.points);
  program->set ("frag_resolution", d.opts.resolution);
  program->set ("frag_nn", d.opts.points);
  program->set ("interval", d.opts.interval);
  program->set ("dash_length", d.opts.dash_length);

  VAID.bind ();

  program->set ("do_lat", 0);
  program->set ("frag_do_lat", 0);
  glDrawArrays (GL_LINE_STRIP, 0, (d.opts.resolution - 1) * (d.opts.points + 1));

  program->set ("do_lat", 1);
  program->set ("frag_do_lat", 1);
  glDrawArrays (GL_LINE_STRIP, 0, 2 * d.opts.resolution * (d.opts.points / 2 + 1));

  VAID.unbind ();

  view.delMVP (program);

  d.labels.render (view, OptionsLight ());

}

}
