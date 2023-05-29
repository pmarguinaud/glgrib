
#include "version.h"

layout (std430, binding=1) buffer vcutLonLat
{
  float lonlat[];
};

layout (std430, binding=2) buffer vcutValue
{
  float values[];
};

layout (std430, binding=3) buffer vcutHeight
{
  float height[];
};

uniform mat4 MVP;
uniform int Nx, Nz;

out float val;

void main()
{
  int i = gl_InstanceID % (Nx - 1);
  int j = gl_VertexID;
  int k = gl_InstanceID / (Nx - 1);

  int di = 0, dk = 0;

  if (j == 0)
    {
      di = 0; dk = 0;
    }
  else if (j == 1)
    {
      di = 1; dk = 0;
    }
  else if (j == 2)
    {
      di = 0; dk = 1;
    }
  else if (j == 3)
    {
      di = 1; dk = 1;
    }

  int ixz = Nx*(k + dk)+(i + di);

  float z = height[ixz];

  val = values[ixz];

  float lon = lonlat[2*(i+di)+0];
  float lat = lonlat[2*(i+di)+1];

  float coslon = cos (lon), sinlon = sin (lon);
  float coslat = cos (lat), sinlat = sin (lat);

  vec3 vertexPos = vec3 (coslon * coslat, sinlon * coslat, sinlat);

  vertexPos = (1 + z) * vertexPos;

  gl_Position = MVP * vec4 (vertexPos, 1);
}
