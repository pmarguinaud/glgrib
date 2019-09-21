#include "glgrib_shapelib.h"
#include "glgrib_dbase.h"
#include "glgrib_resolve.h"

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <shapefil.h>
#include <sqlite3.h>


#include <stdexcept>
#include <iostream>


void glgrib_shapelib::read (const glgrib_options_lines & opts, int * numberOfPoints, 
                            unsigned int * numberOfLines, std::vector<float> * xyz,
                            std::vector<unsigned int> * ind, 
                            const std::string & selector)
{
  const float deg2rad = M_PI / 180.0f;
  bool lonlatsel = (opts.lonmin != 0.0f) 
                || (opts.lonmax != 0.0f)
                || (opts.latmax != 0.0f)
                || (opts.latmax != 0.0f);

  std::string path = glgrib_resolve (opts.path);
  if (path.length () > 4)
    path = path.substr (0, path.length () - 4);

  SHPHandle fp = SHPOpen (path.c_str (), "r");

  glgrib_dbase d;
  d.convert2sqlite (path);

  if (fp == NULL)
    throw std::runtime_error (std::string ("Cannot open Shapefile data : ") + path);

  double minb[4], maxb[4];
  int ntype, nentities;
  SHPGetInfo (fp, &nentities, &ntype, minb, maxb);

  std::vector<int> list;

  if (selector != "")
    {
      std::string sql = "SELECT rowid FROM dbase WHERE " + selector + ";";

      sqlite3 * db = NULL;
      sqlite3_stmt * req = NULL;
      int rc;

#define TRY(expr) do { if ((rc = expr) != SQLITE_OK) goto end; } while (0)
      TRY (sqlite3_open ((path + ".db").c_str (), &db));
      TRY (sqlite3_prepare_v2 (db, sql.c_str (), -1, &req, 0));

      while (1)
        {
          rc = sqlite3_step (req);
          switch (rc)
	    {
              case SQLITE_ROW:
                list.push_back (sqlite3_column_int (req, 0)-1);
              break;
	      case SQLITE_DONE:
	        rc = SQLITE_OK;
              default:
	        goto end;
	    }
	}
end:
      if (rc != SQLITE_OK)
        throw std::runtime_error (std::string (sqlite3_errmsg (db)));
      if (req != NULL)
        sqlite3_finalize (req);
      if (db != NULL)
        sqlite3_close (db);
#undef TRY
    }
  else
    {
      for (int i = 0; i < nentities; i++)
        list.push_back (i);
    }

  auto restart = [ind] 
  { 
    int n = ind->size (); 
    if ((n > 0) && ((*ind)[n-1] != 0xffffffff))
      ind->push_back (0xffffffff);
  };

  int ip = 0;
  for (int k = 0; k < list.size (); k++)
    {
      int i = list[k];

      SHPObject * shape = SHPReadObject(fp, i);

      if (shape == NULL)
        throw std::runtime_error (std::string ("Cannot read record Shapefile data : ") + path);
      

      for (int j = 0, ipart = 0; j < shape->nVertices; j++)
        {
          const char * type;
          bool newpart = false;

          if (j == 0 && shape->nParts > 0)
            {
              type = SHPPartTypeName (shape->panPartType[ipart]);
              ipart++;
              newpart = true;
            }
          
          if ((ipart < shape->nParts) && (shape->panPartStart[ipart] == j))
            {
              type = SHPPartTypeName (shape->panPartType[ipart]);
              ipart++;
              newpart = true;
            }

          if (newpart)
            restart ();

          bool add = true;

          if (lonlatsel)
            {
              float lon = shape->padfX[j];
              float lat = shape->padfY[j];
              bool inlat = (opts.latmin <= lat) && (lat <= opts.latmax);
              bool inlon = false;
              if (inlat)
                {
                  for (int i = -1; i <= 1; i++)
                    {
                      inlon = inlon || ((opts.lonmin <= (lon + i * 360.0f)) 
                                    && ((lon + i * 360.0f) <= opts.lonmax));
                    }
                }
              add = inlat && inlon;
            }

          if (add)
            {
              float lon = shape->padfX[j] * deg2rad; 
              float lat = shape->padfY[j] * deg2rad;

              float coslon = cos (lon);
              float sinlon = sin (lon);
              float coslat = cos (lat);
              float sinlat = sin (lat);

              xyz->push_back (coslon * coslat);
              xyz->push_back (sinlon * coslat);
              xyz->push_back (         sinlat);
              ind->push_back (ip);

              ip++;
            }
          else
            {
              restart ();
            }

        }
    

      SHPDestroyObject (shape);


    }

  SHPClose (fp);

  *numberOfPoints = xyz->size () / 3;
  *numberOfLines  = ind->size ();


}

