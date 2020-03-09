
#include "version.h"

out
#include "GRID_VS.h"

uniform mat4 MVP;
uniform bool do_alpha = false;
uniform float posmax = 0.97;
uniform float scale = 1.005;
uniform int resolution = 10;
uniform int nn = 100;
uniform int do_lat = 0;
uniform int interval = 4;

#include "schmidt.h"
#include "projection.h"

void main()
{
  vec3 pos;

  int ilat, ilon, nlat, nlon;
  if (do_lat == 0)
    {
      nlat = resolution;
      nlon = nn;
      ilat = gl_VertexID / (nn + 1) + 1;
      ilon = gl_VertexID % (nn + 1);

      int jlat = nlat / 2;
      if ((2 * (nlat / 2) != nlat) && (ilat > nlat / 2))
        jlat = jlat + 1;

      grid_vs.dashed = (abs (ilat - jlat) % interval) != 0 ? 1.0f : 0.0f;
    }
  else
    {
      nlon = resolution * 2;
      nlat = nn / 2;
      ilon = gl_VertexID / (nn / 2 + 1);
      ilat = gl_VertexID % (nn / 2 + 1);
      grid_vs.dashed = (ilon % interval) != 0 ? 1.0f : 0.0f;
    }


  float lon = 2.0f * pi * float (ilon) / float (nlon);
  float lat = 0.5f * pi * float (ilat) / float (nlat / 2.0f) - 0.5 * pi;
  float coslon = cos (lon), sinlon = sin (lon);
  float coslat = cos (lat), sinlat = sin (lat);

  grid_vs.vertexPos = vec3 (coslon * coslat, sinlon * coslat, sinlat);

  if (proj == XYZ)
    {
      pos = scale * applySchmidt (grid_vs.vertexPos);
    }
  else
    {
      vec3 normedPos = compNormedPos (grid_vs.vertexPos);
      pos = compProjedPos (grid_vs.vertexPos, normedPos);

      if ((proj == LATLON) || (proj == MERCATOR))
      if ((pos.y < -posmax) || (+posmax < pos.y))
        {
          pos.x = -0.1;
	}

      if (proj == POLAR_SOUTH)
        pos.x = pos.x - 0.005;
      else
        pos.x = pos.x + 0.005;
    }

  gl_Position =  MVP * vec4 (pos, 1.);


}
