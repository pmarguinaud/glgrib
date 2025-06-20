#include "glGrib/ShapeLib.h"
#include "glGrib/Trigonometry.h"
#include "glGrib/DBase.h"
#include "glGrib/Resolve.h"
#include "glGrib/SQLite.h"
#include "glGrib/OpenGL.h"

#include <cmath>
#include <shapefil.h>


#include <stdexcept>
#include <iostream>


namespace glGrib
{

void ShapeLib::read 
  (const OptionsLines & opts, std::vector<float> * lonlat,
   std::vector<unsigned int> * ind, const std::string & selector)
{
  bool lonlatsel = (opts.lonmin != 0.0f) 
                || (opts.lonmax != 0.0f)
                || (opts.latmax != 0.0f)
                || (opts.latmax != 0.0f);

  std::string path = Resolve (opts.path);
  if (path.length () > 4)
    path = path.substr (0, path.length () - 4);

  SHPHandle fp = SHPOpen (path.c_str (), "r");

  DBase d;
  d.convert2sqlite (path);

  if (fp == nullptr)
    throw std::runtime_error (std::string ("Cannot open Shapefile data : ") + path);

  double minb[4], maxb[4];
  int ntype, nentities;
  SHPGetInfo (fp, &nentities, &ntype, minb, maxb);

  std::vector<int> list;

  if (selector != "")
    {
      std::string sql = "SELECT rowid FROM dbase WHERE " + selector + ";";

      SQLite db (path + ".db");
      SQLite::stmt st = db.prepare (sql);

      int k;
      while (st.fetchRow (&k))
        list.push_back (k-1);

    }
  else
    {
      for (int i = 0; i < nentities; i++)
        list.push_back (i);
    }

  auto restart = [ind] 
  { 
    int n = ind->size (); 
    if ((n > 0) && ((*ind)[n-1] != OpenGL::restart))
      ind->push_back (OpenGL::restart);
  };

  int ip = 0;
  for (size_t k = 0; k < list.size (); k++)
    {
      int i = list[k];

      SHPObject * shape = SHPReadObject(fp, i);

      if (shape == nullptr)
        throw std::runtime_error (std::string ("Cannot read record Shapefile data : ") + path);
      

      for (int j = 0, ipart = 0; j < shape->nVertices; j++)
        {
          bool newpart = false;

          if (j == 0 && shape->nParts > 0)
            {
              SHPPartTypeName (shape->panPartType[ipart]);
              ipart++;
              newpart = true;
            }
          
          if ((ipart < shape->nParts) && (shape->panPartStart[ipart] == j))
            {
              SHPPartTypeName (shape->panPartType[ipart]);
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

              lonlat->push_back (lon);
              lonlat->push_back (lat);
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

}

}
