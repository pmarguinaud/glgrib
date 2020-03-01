#include "glGribCities.h"
#include "glGribTrigonometry.h"
#include "glGribResolve.h"
#include "glGribSqlite.h"

#include <iostream>
#include <vector>
#include <stdlib.h>


void glGribCities::setup (const glGribOptionsCities & o)
{
  opts = o;

  std::vector<float> lon, lat, siz;

  std::vector<float> X, Y, Z, A;
  std::vector<std::string> Str;


  glGribSqlite db (glGribResolve ("glGrib.db"));
  glGribSqlite::stmt st;
 
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


  glGribPoints::setup (opts.points, lon, lat, siz);

  d.labels.setShared (true);
  d.labels.setChange (false);

  glgrib_font_ptr font = newGlgribFontPtr (opts.labels.font);

  d.labels.setup3D (font, Str, X, Y, Z, A,
                   opts.labels.font.scale, glGribString::C);
  d.labels.setForegroundColor (opts.labels.font.color.foreground);
  d.labels.setBackgroundColor (opts.labels.font.color.background);


}

glGribCities & glGribCities::operator= (const glGribCities & cities)
{
  if (this != &cities)
    {
      clear ();
      glGribPoints::operator= (cities);
      if (cities.isReady ())
        {
          opts = cities.opts;
          d = cities.d;
	}
    }
  return *this;
}

void glGribCities::clear ()
{
  glGribPoints::clear ();
  d.labels.clear ();
}

void glGribCities::render (const glGribView & view, const glGribOptionsLight & light) const
{
  glGribPoints::render (view, light);
  d.labels.render (view);
}
