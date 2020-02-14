#include "glgrib_cities.h"
#include "glgrib_trigonometry.h"
#include "glgrib_resolve.h"
#include "glgrib_sqlite.h"

#include <iostream>
#include <vector>
#include <stdlib.h>


void glgrib_cities::setup (const glgrib_options_cities & o)
{
  opts = o;

  std::vector<float> lon, lat, siz;

  std::vector<float> X, Y, Z, A;
  std::vector<std::string> Str;


  glgrib_sqlite db (glgrib_resolve ("glgrib.db"));
  glgrib_sqlite::stmt st;
 
  st = db.prepare ("SELECT max (size) FROM CITIES;");

  float maxsize;
  if (! st.fetch_row (&maxsize))
    return;

  st = db.prepare ("SELECT name, lon, lat, size FROM CITIES;");

  float lon_, lat_;
  int siz_;
  std::string name;
  while (st.fetch_row (&name, &lon_, &lat_, &siz_))
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


  glgrib_points::setup (opts.points, lon, lat, siz);

  d.labels.setShared (true);
  d.labels.setChange (false);

  glgrib_font_ptr font = new_glgrib_font_ptr (opts.labels.font);

  d.labels.setup3D (font, Str, X, Y, Z, A,
                   opts.labels.font.scale, glgrib_string::C);
  d.labels.setForegroundColor (opts.labels.font.color.foreground);
  d.labels.setBackgroundColor (opts.labels.font.color.background);


}

glgrib_cities & glgrib_cities::operator= (const glgrib_cities & cities)
{
  if (this != &cities)
    {
      clear ();
      glgrib_points::operator= (cities);
      if (cities.isReady ())
        {
          opts = cities.opts;
          d = cities.d;
	}
    }
  return *this;
}

void glgrib_cities::clear ()
{
  glgrib_points::clear ();
  d.labels.clear ();
}

void glgrib_cities::render (const glgrib_view & view, const glgrib_options_light & light) const
{
  glgrib_points::render (view, light);
  d.labels.render (view);
}
