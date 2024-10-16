#include "glGrib/GSHHG.h"
#include "glGrib/Trigonometry.h"
#include "glGrib/Resolve.h"
#include "glGrib/OpenGL.h"

#include <cmath>
#include <cstring>

#include <stdexcept>
#include <fstream>
#include <iostream>

namespace glGrib
{

namespace
{

void iswap (void * _a, const void * _b, int t, int n, int d)
{
  char * a = (char*)_a;
  const char * b = (const char *)_b;
  int i, j;

  if (d)
    {
      for (i = 0; i < n; i++)
        for (j = 0; j < t / 2; j++)
          {
            char c = b[i*t+j]; /* a and b may be the same */
            a[i*t+j] = b[i*t+t-j-1];
            a[i*t+t-j-1] = c;
          }
    }
  else if (a != b)
    {
      size_t nbytes = t * n;
      memcpy (a, b, nbytes);
    }

}

class GSHHG_t
{                 /* Global Self-consistent Hierarchical High-resolution Shorelines */
public:
  int id;         /* Unique polygon id number, starting at 0 */
  int n;          /* Number of points in this polygon */
  int flag;       /* = level + version << 8 + greenwich << 16 + source << 24 + river << 25 */
  /* flag contains 5 items, as follows:
   * low byte:    level = flag & 255: Values: 1 land, 2 lake, 3 island_in_lake, 4 pond_in_island_in_lake
   * 2nd byte:    version = (flag >> 8) & 255: Values: Should be 12 for GSHHG release 12 (i.e., version 2.2)
   * 3rd byte:    greenwich = (flag >> 16) & 1: Values: Greenwich is 1 if Greenwich is crossed
   * 4th byte:    source = (flag >> 24) & 1: Values: 0 = CIA WDBII, 1 = WVS
   * 4th byte:    river = (flag >> 25) & 1: Values: 0 = not set, 1 = river-lake and level = 2
   */
  int west, east, south, north;   /* min/max extent in micro-degrees */
  int area;       /* Area of polygon in 1/10 km^2 */
  int area_full;  /* Area of original full-resolution polygon in 1/10 km^2 */
  int container;  /* Id of container polygon that encloses this polygon (-1 if none) */
  int ancestor;   /* Id of ancestor polygon in the full resolution set that was the source of this polygon (-1 if none) */

  int read (std::ifstream & fh)
  {
    if (fh.read (reinterpret_cast<char*> (this), sizeof (*this)))
      {
        iswap (this, this, sizeof (int), sizeof (*this)/sizeof (int), 1);
        return 1;
      }
    return 0;
  }

};

class GSHHG_POINT_t
{                 /* Each lon, lat pair is stored in micro-degrees in 4-byte signed integer format */
public:
  int32_t x;
  int32_t y;
};

int read_GSHHG_POINT_list (std::vector<GSHHG_POINT_t> * gpl, int n, std::ifstream & fh)
{
  gpl->resize (n);
  void * ptr = &(*gpl)[0];
  memset (ptr, 0, sizeof (GSHHG_POINT_t) * n);
  if (fh.read (reinterpret_cast<char*> (ptr), n * sizeof (GSHHG_POINT_t)))
    {
      iswap (ptr, ptr, sizeof (int32_t), 2 * n, 1);
      return n;
    }
  return 0;
}

}

void GSHHG::read (const OptionsLines & opts, 
		  std::vector<float> * lonlat,
		  std::vector<unsigned int> * ind, 
                  const std::vector<unsigned int> & mask, 
                  const std::vector<unsigned int> & code)
{
  std::string path = Resolve (opts.path);

  GSHHG_t h;
  std::vector<GSHHG_POINT_t> gpl;
  bool lonlatsel = (opts.lonmin != 0.0f) 
                || (opts.lonmax != 0.0f)
                || (opts.latmax != 0.0f)
                || (opts.latmax != 0.0f);

  int ip = 0;

  const float microdeg2rad = pi / (1000000. * 180.);

  std::ifstream fh (path, std::ios::in | std::ifstream::binary);

  if (! fh)
    throw std::runtime_error (std::string ("Cannot open GSHHG data"));
  
  auto restart = [ind] 
  { 
    int n = ind->size (); 
    if ((n > 0) && ((*ind)[n-1] != OpenGL::restart))
      ind->push_back (OpenGL::restart);
  };

  while (1) 
    {   
      if (! h.read (fh))
        break;

      if (! read_GSHHG_POINT_list (&gpl, h.n, fh))
        break;

      bool ok = false;

      for (size_t i = 0; i < mask.size (); i++)
        ok = ok || ((h.flag & mask[i]) == code[i]);
      
      if (ok)
        {
          for (int i = 0; i < h.n; i++)
            {
              bool add = true;

              if (lonlatsel)
                {
                  float lon = gpl[i].x / 1000000.0f;
                  float lat = gpl[i].y / 1000000.0f;
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
                  float lon = microdeg2rad * gpl[i].x;
    	          float lat = microdeg2rad * gpl[i].y;

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
          ind->push_back (OpenGL::restart);
        }

      if (fh.eof ())
        break;

    }   

      
}

}
