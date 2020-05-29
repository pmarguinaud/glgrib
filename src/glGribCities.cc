#include "glGribCities.h"
#include "glGribTrigonometry.h"
#include "glGribResolve.h"
#include "glGribSQLite.h"

#include <iostream>
#include <vector>
#include <stdlib.h>


void glGrib::Cities::setup (const glGrib::OptionsCities & o)
{
  opts = o;

  std::vector<float> lon, lat, siz;

  std::vector<float> X, Y, Z, A;
  std::vector<std::string> Str;


  glGrib::SQLite db (glGrib::Resolve ("glGrib.db"));
  glGrib::SQLite::stmt st;
 
  st = db.prepare ("SELECT max (size) FROM CITIES;");

  float maxsize;
  if (! st.fetchRow (&maxsize))
    return;

  st = db.prepare ("SELECT name, lon, lat, size FROM CITIES;");

  float lon_, lat_;
  int siz_;
  std::string name;
  while (st.fetchRow (&name, &lon_, &lat_, &siz_))
    {
      lon.push_back (lon_);
      lat.push_back (lat_);
      siz.push_back (log (siz_) / log (10.0));
      if ((siz_ > 1000000) && (opts.labels.on))
        {
          Str.push_back (std::string (name));
	  float x, y, z;
	  lonlat2xyz (deg2rad * lon_, deg2rad * lat_, &x, &y, &z);
          X.push_back (x * opts.points.scale * 1.01);
          Y.push_back (y * opts.points.scale * 1.01);
          Z.push_back (z * opts.points.scale * 1.01);
          A.push_back (0.0f);
       }
    }


  glGrib::Points::setup (opts.points, lon, lat, siz);

  d.labels.setShared (true);
  d.labels.setChange (false);

  glGrib::FontPtr font = newGlgribFontPtr (opts.labels.font);

  d.labels.setup3D (font, Str, X, Y, Z, A,
                   opts.labels.font.scale, glGrib::String::C);
  d.labels.setForegroundColor (opts.labels.font.color.foreground);
  d.labels.setBackgroundColor (opts.labels.font.color.background);


}

void glGrib::Cities::clear ()
{
  glGrib::Points::clear ();
  d.labels.clear ();
}

void glGrib::Cities::render (const glGrib::View & view, const glGrib::OptionsLight & light) const
{
  glGrib::Points::render (view, light);
  d.labels.render (view);
}
