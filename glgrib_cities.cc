#include "glgrib_cities.h"
#include "glgrib_resolve.h"

#include <iostream>
#include <vector>
#include <stdlib.h>
#include <sqlite3.h>


void glgrib_cities::init (const glgrib_options_cities & o)
{
  const double deg2rad = M_PI / 180.0;

  opts = o;

  std::vector<float> lon, lat, siz;

  std::vector<float> X, Y, Z, A;
  std::vector<std::string> Str;

  float maxsize;

  sqlite3 * db = NULL;
  sqlite3_stmt * req = NULL;
  int rc;

#define TRY(expr) do { if ((rc = expr) != SQLITE_OK) goto end; } while (0)

  TRY (sqlite3_open (glgrib_resolve (std::string ("glgrib.db")).c_str (), &db));
  TRY (sqlite3_prepare_v2 (db, "SELECT max (size) FROM CITIES;", -1, &req, 0));
  if ((rc = sqlite3_step (req)) != SQLITE_ROW)
    goto end;
  rc = SQLITE_OK;
  maxsize = sqlite3_column_double (req, 0);
  sqlite3_finalize (req);
  req = NULL;

  TRY (sqlite3_prepare_v2 (db, "SELECT name, lon, lat, size FROM CITIES;", -1, &req, 0));

  while (1)
    {
      rc = sqlite3_step (req);
      switch (rc)
        {
          case SQLITE_ROW:
            {
              const char * name = (const char *)sqlite3_column_text (req, 0);
	      float lon_ = sqlite3_column_double (req, 1);
	      float lat_ = sqlite3_column_double (req, 2);
	      float siz_ = sqlite3_column_int    (req, 3);
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
	      break;
	    }
	  case SQLITE_DONE:
	    rc = SQLITE_OK;
	  default:
            goto end;
	}
    }

#undef TRY

end:

  if (rc != SQLITE_OK)
    throw std::runtime_error (std::string (sqlite3_errmsg (db)));

  if (req != NULL)
    sqlite3_finalize (req);
  if (db != NULL)
    sqlite3_close (db);

  glgrib_points::init (opts.points, lon, lat, siz);

  d.labels.setShared (true);
  d.labels.setChange (false);

  glgrib_font_ptr font = new_glgrib_font_ptr (opts.labels.font);

#ifdef UNDEF
  d.labels.init3D (font, std::vector<std::string>{"ABCD","EFGH","IJKL","MNOP"},
                   std::vector<float>{+1.01f,-1.01f,+0.00f,+0.707*1.010f},
                   std::vector<float>{+0.00f,+0.00f,+1.01f,+0.707*0.000f},
                   std::vector<float>{+0.00f,+0.00f,+0.00f,+0.707*1.010f},
                   std::vector<float>{+0.0f,+0.0f,+90.0f,+0.0f},
                   opts.labels.font.scale, glgrib_string::C);
#else
  d.labels.init3D (font, Str, X, Y, Z, A,
                   opts.labels.font.scale, glgrib_string::C);
  d.labels.setForegroundColor (opts.labels.font.color.foreground);
  d.labels.setBackgroundColor (opts.labels.font.color.background);
#endif


}

glgrib_cities & glgrib_cities::operator= (const glgrib_cities & cities)
{
  if (this != &cities)
    {
      cleanup ();
      glgrib_points::operator= (cities);
      if (cities.isReady ())
        {
          opts = cities.opts;
          d = cities.d;
	}
    }
}

void glgrib_cities::cleanup ()
{
  glgrib_points::cleanup ();
  d.labels.cleanup ();
}

void glgrib_cities::render (const glgrib_view & view) const
{
  glgrib_points::render (view);
  d.labels.render (view);
}
