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


void glgrib_shapelib::read (const std::string & path, int * numberOfPoints, 
                            unsigned int * numberOfLines, std::vector<float> * xyz,
                            std::vector<unsigned int> * ind, 
                            const std::string & selector)
{
  const float deg2rad = M_PI / 180.0f;

  std::string p = glgrib_resolve (path);
  if (p.length () > 4)
    p = p.substr (0, p.length () - 4);

  SHPHandle fp = SHPOpen (p.c_str (), "r");

  glgrib_dbase d;
  d.convert2sqlite (p);

  if (fp == NULL)
    throw std::runtime_error (std::string ("Cannot open Shapefile data : ") + path);

  double minb[4], maxb[4];
  int ntype, nentities;
  SHPGetInfo (fp, &nentities, &ntype, minb, maxb);

  *numberOfLines = 0;
  *numberOfPoints = 0;

  std::vector<int> list;

  if (selector != "")
    {
      std::string sql = "SELECT rowid FROM dbase WHERE " + selector + ";";

      sqlite3 * db = NULL;
      sqlite3_stmt * req = NULL;
      int rc;

#define TRY(expr) do { if ((rc = expr) != SQLITE_OK) goto end; } while (0)
      TRY (sqlite3_open ((p + ".db").c_str (), &db));
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

  int count = -1;
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


          float lon = shape->padfX[j] * deg2rad; 
          float lat = shape->padfY[j] * deg2rad;

          float coslon = cos (lon);
          float sinlon = sin (lon);
          float coslat = cos (lat);
          float sinlat = sin (lat);

          xyz->push_back (coslon * coslat);
          xyz->push_back (sinlon * coslat);
          xyz->push_back (         sinlat);

          (*numberOfPoints)++;

          if (newpart)
            {
              count++;
            }
          else
            {
              ind->push_back (count); count++;
              ind->push_back (count);
              (*numberOfLines)++;
            }


        }
    

      SHPDestroyObject (shape);


    }

  SHPClose (fp);


}

