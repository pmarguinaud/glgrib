
#include "version.h"

layout (std430, binding=1) buffer vcutLonLat
{
  float lonlat[];
};

uniform mat4 MVP;

void main()
{
  int i = gl_InstanceID;
  int j = gl_VertexID;

  int di = 0, dj = 0;

  if (j == 0)
    {
      di = 0;
      dj = 0;
    }
  else if (j == 1)
    {
      di = 1;
      dj = 0;
    }
  else if (j == 2)
    {
      di = 0;
      dj = 1;
    }
  else if (j == 3)
    {
      di = 1;
      dj = 1;
    }


  float lon = lonlat[2*(i+di)+0];
  float lat = lonlat[2*(i+di)+1];

  float coslon = cos (lon), sinlon = sin (lon);
  float coslat = cos (lat), sinlat = sin (lat);

  vec3 vertexPos = vec3 (coslon * coslat, sinlon * coslat, sinlat);

  if (dj == 1)
    {
      vertexPos = 2 * vertexPos;
    }

  gl_Position = MVP * vec4 (vertexPos, 1);
}
