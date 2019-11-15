#include "glgrib_cities.h"
#include "glgrib_resolve.h"
#include "glgrib_sqlite.h"

#include <iostream>
#include <vector>
#include <stdlib.h>


void glgrib_cities::setup (const glgrib_options_cities & o)
{
  const double deg2rad = M_PI / 180.0;

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
          float coslon = cos (deg2rad * lon_), sinlon = sin (deg2rad * lon_);
          float coslat = cos (deg2rad * lat_), sinlat = sin (deg2rad * lat_);
          float x = coslon * coslat * opts.points.scale * 1.01;
          float y = sinlon * coslat * opts.points.scale * 1.01;
          float z =          sinlat * opts.points.scale * 1.01;
          X.push_back (x);
          Y.push_back (y);
          Z.push_back (z);
          A.push_back (0.0f);
       }
    }


  glgrib_points::setup (opts.points, lon, lat, siz);

  d.labels.setShared (true);
  d.labels.setChange (false);

  glgrib_font_ptr font = new_glgrib_font_ptr (opts.labels.font);

#ifdef UNDEF
  d.labels.setup3D (font, std::vector<std::string>{"ABCD","EFGH","IJKL","MNOP"},
                   std::vector<float>{+1.01f,-1.01f,+0.00f,+0.707*1.010f},
                   std::vector<float>{+0.00f,+0.00f,+1.01f,+0.707*0.000f},
                   std::vector<float>{+0.00f,+0.00f,+0.00f,+0.707*1.010f},
                   std::vector<float>{+0.0f,+0.0f,+90.0f,+0.0f},
                   opts.labels.font.scale, glgrib_string::C);
#else
  d.labels.setup3D (font, Str, X, Y, Z, A,
                   opts.labels.font.scale, glgrib_string::C);
  d.labels.setForegroundColor (opts.labels.font.color.foreground);
  d.labels.setBackgroundColor (opts.labels.font.color.background);
#endif


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

void glgrib_cities::render (const glgrib_view & view) const
{
  glgrib_points::render (view);
  d.labels.render (view);
}
