#pragma once

#include "glGrib/Geometry.h"

namespace glGrib
{

namespace Contour
{

template <typename ISO, typename VAL>
void processTriangle
  (int it0, const VAL & val, float val0, bool * seen, 
   ISO * iso, const const_GeometryPtr & geometry)
{
  int count = 0;
  bool cont = true;
  bool edge = false;
  int it = it0;
  int its[2];

  float vv[3];

  iso->start ();

  while (cont)
    {
      cont = false;

      if (seen[it])
        break;

      seen[it] = true;

      int jglo[3], itri[3];
      glm::vec3 xyz[3];

      geometry->getTriangleNeighbours (it, jglo, itri, xyz);

      // Get values for current triangle
      for (int i = 0; i < 3; i++)
        vv[i] = val(jglo[i]);

      int n = 0;
      for (int i = 0; i < 3; i++)
        if (vv[i] < val0)
          n++;

      if ((n == 0) || (n == 3)) // 3 vertices have the same color
        break;

      if (count == 0) // First triangle; see if it is at the edge of the domain
        {
          int c = 0;
          for (int i = 0; i < 3; i++)
            {
              int iA = i, iB = (i + 1) % 3;
              bool bA = vv[iA] < val0, bB = vv[iB] < val0;
              int itAB = itri[iA];
              if ((bA != bB) && (! seen[itAB]))
                c++;
            }
          edge = c != 2;
          if ((! edge) && geometry->triangleIsEdge (it))
            {
              seen[it] = false;
              return;
            }

        }

      // Find a way out of current triangle
      for (int i = 0; i < 3; i++)
        {
          int iA = i, iB = (i + 1) % 3;
          int jgloA = jglo[iA], jgloB = jglo[iB];
	  float valA = vv[iA], valB = vv[iB];
	  bool bA = valA < val0, bB = valB < val0;
          int itAB = itri[iA];
          if ((bA != bB) && (! seen[itAB]))
            {
              bool lswap = jgloA > jgloB;
              if (lswap)
                {
                  std::swap (jgloA, jgloB);
                  std::swap (valA, valB);
                  std::swap (iA, iB);
                }
              float a = (val0 - valA) / (valB - valA);

              iso->push (xyz[iA], xyz[iB], jgloA, jgloB, a);

              if (count < 2)
                its[count] = it;

              it = itAB;

              count++;
              cont = true;
              break;
            }
        }

      // Reset back seen array to false for first two triangles, so that contour lines be closed
      if ((count == 2) && (! edge))
        seen[its[0]] = false;
      if ((count == 3) && (! edge))
        seen[its[1]] = false;
    }

  if (count > 0)
    iso->close (edge);

  return;
}

template <typename ISO, typename VAL>
void processTriangles (const const_GeometryPtr & geometry, const float level,
                      ISO * iso, const VAL & val)
{
  const int nt = geometry->getNumberOfTriangles ();
  Buffer<bool> seen (nt + 1);

  for (int i = 0; i < nt + 1; i++)
    seen[i] = false;
  seen[0] = true;
  
  // First visit edge triangles (open contours)
  for (int it = 0; it < nt; it++)
    if (geometry->triangleIsEdge (it))
      processTriangle (it, val, level, &seen[1], iso, geometry);
  
  // Second : visit inner triangles (closed contours)
  for (int it = 0; it < nt; it++)
    processTriangle (it, val, level, &seen[1], iso, geometry);
}


}

}
