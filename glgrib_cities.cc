#include "glgrib_cities.h"
#include "glgrib_resolve.h"

#include <iostream>
#include <vector>
#include <stdlib.h>
#include <sqlite3.h>


void glgrib_cities::init (const glgrib_options_cities & o)
{
  opts = o;

  std::vector<float> lon, lat, siz;
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
              lon.push_back (sqlite3_column_double (req, 1));
              lat.push_back (sqlite3_column_double (req, 2));
//            siz.push_back ((float)sqlite3_column_int (req, 3) / 10000000.);
              siz.push_back (1.);
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

  glgrib_points::init (lon, lat, siz);

}

