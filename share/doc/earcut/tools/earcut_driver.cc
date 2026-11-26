// Standalone driver : read a GSHHS shapefile like glGrib::ShapeLib::read,
// and run glGrib::EarCut::processRing on selected rings.
//
// Usage : earcut_driver file.shp [omp|noomp] [rank|all|min=N] ...

#include "glGrib/EarCut.h"

#include <shapefil.h>
#include <omp.h>

#include <vector>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <set>

int main (int argc, char * argv[])
{
  if (argc < 3)
    {
      fprintf (stderr, "Usage: %s file.shp omp|noomp [rank ...|all|min=N]\n", argv[0]);
      return 1;
    }

  std::string path = argv[1];
  path = path.substr (0, path.length () - 4);
  bool openmp = std::string (argv[2]) == "omp";

  std::set<int> ranks;
  bool all = false;
  int nmin = -1;
  for (int i = 3; i < argc; i++)
    {
      std::string a = argv[i];
      if (a == "all")
        all = true;
      else if (a.substr (0, 4) == "min=")
        nmin = std::atoi (a.c_str () + 4);
      else
        ranks.insert (std::atoi (a.c_str ()));
    }

  SHPHandle fp = SHPOpen (path.c_str (), "r");
  if (fp == nullptr)
    {
      fprintf (stderr, "Cannot open %s\n", path.c_str ());
      return 1;
    }

  int nentities, ntype;
  double minb[4], maxb[4];
  SHPGetInfo (fp, &nentities, &ntype, minb, maxb);

  const float deg2rad = M_PI / 180.0;

  std::vector<float> lonlat;
  std::vector<int> off, len;

  for (int i = 0; i < nentities; i++)
    {
      SHPObject * shape = SHPReadObject (fp, i);
      int nparts = shape->nParts > 0 ? shape->nParts : 1;
      for (int p = 0; p < nparts; p++)
        {
          int j1 = shape->nParts > 0 ? shape->panPartStart[p] : 0;
          int j2 = (p + 1 < shape->nParts) ? shape->panPartStart[p+1] : shape->nVertices;
          off.push_back (lonlat.size () / 2);
          len.push_back (j2 - j1 - 1);  // Same as Land::layer_t::triangulate
          for (int j = j1; j < j2; j++)
            {
              lonlat.push_back (shape->padfX[j] * deg2rad);
              lonlat.push_back (shape->padfY[j] * deg2rad);
            }
        }
      SHPDestroyObject (shape);
    }

  SHPClose (fp);

  for (size_t k = 0; k < off.size (); k++)
    {
      bool sel = all || ranks.count (off[k]) || ((nmin > 0) && (len[k] >= nmin));
      if ((! sel) || (len[k] <= 2))
        continue;
      std::vector<unsigned int> ind (3 * (len[k] - 2));
      int indl = ind.size ();
      double t0 = omp_get_wtime ();
      // Same as Land::layer_t::triangulate : OpenMP for rings >= 300 points
      glGrib::EarCut::processRing (lonlat, off[k], len[k], 0, &indl, &ind, openmp && (len[k] >= 300));
      double t1 = omp_get_wtime ();
      printf ("%d %d %d %.6f\n", off[k], len[k], indl / 3, t1 - t0);
    }

  return 0;
}
