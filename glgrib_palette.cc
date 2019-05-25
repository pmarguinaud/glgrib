#include "glgrib_palette.h"
#include "glgrib_opengl.h"

#include <iostream>
#include <fstream>
#include <stdio.h>
#include <string.h>
#include <sqlite3.h>

std::string palette_directory;

typedef std::map<std::string,glgrib_palette> name2palette_t;
static name2palette_t name2palette;

glgrib_palette & glgrib_palette::register_ (const glgrib_palette & p)
{
  name2palette.insert (std::pair<std::string, glgrib_palette>(name, p));
  name2palette_t::iterator it = name2palette.find (name);
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

glgrib_palette & get_palette_by_name (const std::string & name)
{
  name2palette_t::iterator it = name2palette.find (name);

  if (it != name2palette.end ())
    return it->second;

  std::string pp = palette_directory + "/" + name + ".dat";
  std::ifstream fh (pp);
  if (fh.is_open ())
    {
      glgrib_palette p = glgrib_palette (fh);
      p.name = name;
      return p.register_ (p);
    }
    
  return palette_white_black;
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

glgrib_palette & get_next_palette (const glgrib_palette & p)
{
  name2palette_t::iterator it = name2palette.find (p.name);
  if (it != name2palette.end ())
    it++;

  if (it == name2palette.end ())
    it = name2palette.begin ();
  
  return it->second;
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
  if (p1.min != p2.min)
    return false;
  if (p1.max != p2.max)
    return false;
  if (p1.rgba != p2.rgba)
    return false;
  return true;
}
 
bool operator!= (const glgrib_palette & p1, const glgrib_palette & p2)
{
  return ! (p1 == p2);
}
 
glgrib_palette get_palette_by_meta (const glgrib_field_metadata  & meta)
{
  sqlite3 * db = NULL;
  sqlite3_stmt * req = NULL;
  std::string pname = "default";
  float pmin = std::numeric_limits<float>::max ();
  float pmax = std::numeric_limits<float>::min ();
  int rc;

  rc = sqlite3_open (".glgrib.db", &db);

  if (rc != SQLITE_OK) 
    goto end;


  if ((meta.discipline != 255) && (meta.parameterCategory != 255) && (meta.parameterNumber != 255))
    {
      rc = sqlite3_prepare_v2 (db, "SELECT palette, min, max FROM GRIB2PALETTE WHERE discipline = ? "
                                   "AND parameterCategory = ? AND parameterNumber = ?;", -1, &req, 0);    

      if (rc != SQLITE_OK)
        goto end;
     
      rc = sqlite3_bind_int (req, 1, meta.discipline);
      rc = sqlite3_bind_int (req, 2, meta.parameterCategory);
      rc = sqlite3_bind_int (req, 3, meta.parameterNumber);
     
      if (rc != SQLITE_OK)
        goto end;
     
      rc = sqlite3_step (req);
      if (rc == SQLITE_ROW)
        goto step;

      sqlite3_finalize (req);
      req = NULL;
    }

  if (meta.CLNOMA != "")
    {
      rc = sqlite3_prepare_v2 (db, "SELECT palette, min, max FROM CLNOMA2PALETTE WHERE CLNOMA = ?;", -1, &req, 0);    
     
      if (rc != SQLITE_OK)
        goto end;
     
      rc = sqlite3_bind_text (req, 1, meta.CLNOMA.c_str (), meta.CLNOMA.length (), NULL);
     
      if (rc != SQLITE_OK)
        goto end;

      rc = sqlite3_step (req);
      if (rc == SQLITE_ROW)
        goto step;

      sqlite3_finalize (req);
      req = NULL;
    }

  goto end;

step:
  
  char name[32];
  strcpy (name, (const char *)sqlite3_column_text (req, 0));
  pmin = sqlite3_column_double (req, 1);
  pmax = sqlite3_column_double (req, 2);
  pname = std::string (name);

end:
  if (req != NULL)
    sqlite3_finalize (req);
  if (db != NULL)
    sqlite3_close (db);

  glgrib_palette p = get_palette_by_name (pname);
  p.min = pmin;
  p.max = pmax;
  return p;
}


