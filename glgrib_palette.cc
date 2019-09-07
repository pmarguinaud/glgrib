#include "glgrib_palette.h"
#include "glgrib_opengl.h"
#include "glgrib_resolve.h"

#include <iostream>
#include <fstream>
#include <stdio.h>
#include <string.h>
#include <sqlite3.h>

const float glgrib_palette::defaultMin = std::numeric_limits<float>::max();
const float glgrib_palette::defaultMax = std::numeric_limits<float>::min();

typedef std::map<std::string,glgrib_palette> name2palette_t;
static name2palette_t name2palette;

glgrib_palette & glgrib_palette::register_ (const glgrib_palette & p)
{
  name2palette.insert (std::pair<std::string, glgrib_palette>(opts.name, p));
  name2palette_t::iterator it = name2palette.find (opts.name);
  return it->second;
}

static
glgrib_palette palette_white_black
  (
    "white_black",
      0,   0,   0,   0,
      0,   0,   0, 255,
    255, 255, 255, 255
  );

glgrib_palette glgrib_palette::create 
        (const glgrib_options_palette & o,  
         float min, float max,
         const glgrib_field_metadata & meta)
{
  glgrib_palette p;

  if (o.name == "default")
    {
      p = create_by_meta (meta);
    }
  else
    {
      p = create_by_name (o.name);
      p.opts = o;
    }

  if (p.opts.min == defaultMin)
    p.opts.min = min;
  if (p.opts.max == defaultMax)
    p.opts.max = max;

  return p;
}

glgrib_palette & glgrib_palette::create_by_name (const std::string & name)
{
  name2palette_t::iterator it = name2palette.find (name);

  if (it != name2palette.end ())
    return it->second;

  glgrib_palette p;
  bool found = false;

  sqlite3 * db = NULL;
  sqlite3_stmt * req = NULL;
  int rc;
#define TRY(expr) do { if ((rc = expr) != SQLITE_OK) goto end; } while (0)

  TRY (sqlite3_open (glgrib_resolve (std::string ("glgrib.db")).c_str (), &db));
  TRY (sqlite3_prepare_v2 (db, "SELECT hexa FROM PALETTES WHERE name = ?;", -1, &req, 0));
  TRY (sqlite3_bind_text (req, 1, name.c_str (), strlen (name.c_str ()), NULL));

  if ((rc = sqlite3_step (req)) == SQLITE_ROW)
    {
      found = true;
      char hexa[2049];
      strcpy (hexa, (const char *)sqlite3_column_text (req, 0));
      for (int i = 0; i < 256; i++)
        {
          int r, g, b, a;
          if (sscanf (&hexa[8*i], "%2x%2x%2x%2x", &r, &g, &b, &a) != 4)
            throw std::runtime_error ("Cannot parse hexa color");
          if (i == 0)
            p.rgba_mis = glgrib_rgba ((byte)r, (byte)g, (byte)b, (byte)a);
          else
            p.rgba.push_back (glgrib_rgba ((byte)r, (byte)g, (byte)b, (byte)a));
        }
 
      rc = SQLITE_OK;
      found = true;

      p.opts.name = name;
    }
  else
    {
      TRY (sqlite3_finalize (req));
      req = NULL;
    }

#undef TRY

end:

  if (rc != SQLITE_OK)
    throw std::runtime_error (std::string (sqlite3_errmsg (db)));

  if (req != NULL)
    sqlite3_finalize (req);
  if (db != NULL)
    sqlite3_close (db);
    
  return found ? p.register_ (p) : palette_white_black;
}

glgrib_palette::glgrib_palette (std::ifstream & fh)
{
  std::string head;
  std::getline (fh, head);
  if (head[0] == '-')
    {
    }
  int r, g, b, a;
  fh >> r >> g >> b >> a;
  rgba_mis = glgrib_rgba ((byte)r, (byte)g, (byte)b, (byte)a);
  while (fh >> r >> g >> b >> a)
    rgba.push_back (glgrib_rgba ((byte)r, (byte)g, (byte)b, (byte)a));
}

glgrib_palette glgrib_palette::next (const glgrib_palette & p, float min, float max)
{
  name2palette_t::iterator it = name2palette.find (p.opts.name);
  if (it != name2palette.end ())
    it++;

  if (it == name2palette.end ())
    it = name2palette.begin ();
  
  glgrib_palette p1 = it->second;

  p1.opts.min = min;
  p1.opts.max = max;

  return p1;
}

glgrib_palette palette_zsdiff_big
  (
     -350.0f, +350.0f,
     "zsdiff_big",
       0,   0,   0,   0,
       0,   0, 255, 255,
     255, 255, 255, 255,
     255,   0,   0, 255 
  );

glgrib_palette palette_cold_hot_temp
  (
     253.15, 313.15,
     "cold_hot_temp",
       0,   0,   0,   0,
       0,   0, 255, 255,
     255, 255, 255, 255,
     255,   0,   0, 255 
  );

glgrib_palette palette_cold_hot
  (
//   253.15, 293.15,
     "cold_hot",
       0,   0,   0,   0,
       0,   0, 255, 255,
     255, 255, 255, 255,
     255,   0,   0, 255 
  );

glgrib_palette palette_cloud
  (
     0., 100.,
     "cloud",
       0,   0,   0,   0,
     255, 255, 255,   0,
     255, 255, 255, 255
  );

glgrib_palette palette_cloud_auto
  (
     "cloud_auto",
       0,   0,   0,   0,
     255, 255, 255,   0,
     255, 255, 255, 255
  );


std::ostream & operator << (std::ostream &out, const glgrib_palette & p)
{
  out << p.rgba_mis << std::endl;
  out << "[";
  for (std::vector<glgrib_rgba>::const_iterator it = p.rgba.begin (); it != p.rgba.end (); it++)
    out << *it << ",";
  out << "]" << std::endl;
}

void glgrib_palette::setRGBA255 (GLuint programID) const
{
  float RGBA0[256][4];
  int n = rgba.size ();

  RGBA0[0][0] = rgba_mis.r; RGBA0[0][1] = rgba_mis.b;
  RGBA0[0][2] = rgba_mis.g; RGBA0[0][3] = rgba_mis.a;

  for (int j = 0; j < n-1; j++)
    {
      int j0 = j + 0;
      int j1 = j + 1;
      int i0 = 1 + (255 * j0) / (n - 1);
      int i1 = 1 + (255 * j1) / (n - 1);
      for (int i = i0; i < i1; i++)
        {
          float w0 = (float)(i1-i) / (float)(i1-i0);
          float w1 = (float)(i-i0) / (float)(i1-i0);
          RGBA0[i][0] = (rgba[j0].r * w0 + rgba[j1].r * w1) / 255.;
          RGBA0[i][1] = (rgba[j0].g * w0 + rgba[j1].g * w1) / 255.;
          RGBA0[i][2] = (rgba[j0].b * w0 + rgba[j1].b * w1) / 255.;
          RGBA0[i][3] = (rgba[j0].a * w0 + rgba[j1].a * w1) / 255.;
        }
    }

  glUniform4fv (glGetUniformLocation (programID, "RGBA0"), 256, &RGBA0[0][0]);
}

bool operator== (const glgrib_palette & p1, const glgrib_palette & p2)
{
  if (p1.rgba_mis != p2.rgba_mis)
    return false;
  if (p1.opts.min != p2.opts.min)
    return false;
  if (p1.opts.max != p2.opts.max)
    return false;
  if (p1.rgba != p2.rgba)
    return false;
  return true;
}
 
bool operator!= (const glgrib_palette & p1, const glgrib_palette & p2)
{
  return ! (p1 == p2);
}
 
glgrib_palette glgrib_palette::create_by_meta (const glgrib_field_metadata  & meta)
{
  sqlite3 * db = NULL;
  sqlite3_stmt * req = NULL;
  std::string pname = "default";
  float pmin = std::numeric_limits<float>::max ();
  float pmax = std::numeric_limits<float>::min ();
  int rc;

#define TRY(expr) do { if ((rc = expr) != SQLITE_OK) goto end; } while (0)
  TRY (sqlite3_open (glgrib_resolve ("glgrib.db").c_str (), &db));

  if ((meta.discipline != 255) && (meta.parameterCategory != 255) && (meta.parameterNumber != 255))
    {
      TRY (sqlite3_prepare_v2 (db, "SELECT palette, min, max FROM GRIB2PALETTE WHERE discipline = ? "
                                   "AND parameterCategory = ? AND parameterNumber = ?;", -1, &req, 0));

      TRY (sqlite3_bind_int (req, 1, meta.discipline));
      TRY (sqlite3_bind_int (req, 2, meta.parameterCategory));
      TRY (sqlite3_bind_int (req, 3, meta.parameterNumber));
     
      if ((rc = sqlite3_step (req)) == SQLITE_ROW)
        goto step;

      TRY (sqlite3_finalize (req));
      req = NULL;
    }

  if (meta.CLNOMA != "")
    {
      TRY (sqlite3_prepare_v2 (db, "SELECT palette, min, max "
                                   "FROM CLNOMA2PALETTE WHERE CLNOMA = ?;", -1, &req, 0));
     
      TRY (sqlite3_bind_text (req, 1, meta.CLNOMA.c_str (), meta.CLNOMA.length (), NULL));

      if ((rc = sqlite3_step (req)) == SQLITE_ROW)
        goto step;

      TRY (sqlite3_finalize (req));
      req = NULL;
    }

  goto end;
#undef TRY

step:
  
  char name[32];
  strcpy (name, (const char *)sqlite3_column_text (req, 0));
  pmin = sqlite3_column_double (req, 1);
  pmax = sqlite3_column_double (req, 2);
  pname = std::string (name);
  rc = SQLITE_OK;

end:

  if (rc != SQLITE_OK)
    throw std::runtime_error (std::string (sqlite3_errmsg (db)));

  if (req != NULL)
    sqlite3_finalize (req);
  if (db != NULL)
    sqlite3_close (db);



  glgrib_palette p = glgrib_palette::create_by_name (pname);
  p.opts.min = pmin;
  p.opts.max = pmax;
  return p;
}

void glgrib_palette::save (const glgrib_field_metadata & meta) const
{
  sqlite3 * db = NULL;
  sqlite3_stmt * req = NULL;
  int rc;

#define TRY(expr) do { if ((rc = expr) != SQLITE_OK) goto end; } while (0)

  TRY (sqlite3_open (glgrib_resolve ("glgrib.db").c_str (), &db));

  if ((meta.discipline != 255) && (meta.parameterCategory != 255) && (meta.parameterNumber != 255))
    {
      TRY (sqlite3_prepare_v2 (db, "INSERT OR REPLACE INTO GRIB2PALETTE "
                                   "VALUES (?, ?, ?, ?, ?, ?);", -1, &req, 0));
      TRY (sqlite3_bind_int    (req, 1, meta.discipline));
      TRY (sqlite3_bind_int    (req, 2, meta.parameterCategory));
      TRY (sqlite3_bind_int    (req, 3, meta.parameterNumber));
      TRY (sqlite3_bind_double (req, 4, opts.min));
      TRY (sqlite3_bind_double (req, 5, opts.max));
      TRY (sqlite3_bind_text   (req, 6, opts.name.c_str (), opts.name.length (), NULL));
     
      if ((rc = sqlite3_step (req)) != SQLITE_DONE)
        goto end;

      TRY (sqlite3_finalize (req));
      req = NULL;
    }

  if (meta.CLNOMA != "")
    {
      TRY (sqlite3_prepare_v2 (db, "INSERT OR REPLACE CLNOMA, palette, min, "
                                   "max FROM CLNOMA2PALETTE;", -1, &req, 0));
      TRY (sqlite3_bind_text (req, 1, meta.CLNOMA.c_str (), meta.CLNOMA.length (), NULL));
 
      if ((rc = sqlite3_step (req)) != SQLITE_DONE)
        goto end;

      TRY (sqlite3_finalize (req));
      req = NULL;
    }
#undef TRY

end:

  if (rc != SQLITE_OK)
    throw std::runtime_error (std::string (sqlite3_errmsg (db)));

  if (req != NULL)
    sqlite3_finalize (req);
  if (db != NULL)
    sqlite3_close (db);

}



