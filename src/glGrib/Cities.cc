#include "glGrib/Cities.h"
#include "glGrib/Trigonometry.h"
#include "glGrib/Resolve.h"
#include "glGrib/SQLite.h"

#include <iostream>
#include <vector>

namespace glGrib
{

void Cities::setup (const OptionsCities & o)
{
  if (! o.on)
    return;

  opts = o;

  std::vector<float> lon, lat, siz;

  std::vector<float> X, Y, Z, A;
  std::vector<std::string> Str;


  SQLite db (Resolve ("glGrib.db"));
  SQLite::stmt st;
 
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
      siz.push_back (std::log (siz_) / std::log (10.0));
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


  Points::setup (opts.points, lon, lat, siz);

  FontPtr font = getGlGribFontPtr (opts.labels.font);

  d.labels.setup (font, Str, X, Y, Z, A,
                  opts.labels.font.bitmap.scale, StringTypes::C);
  d.labels.setForegroundColor (opts.labels.font.color.foreground);
  d.labels.setBackgroundColor (opts.labels.font.color.background);


}

void Cities::render (const View & view, const OptionsLight & light) const
{
  Points::render (view, light);
  d.labels.render (view, OptionsLight ());
}

}
