
#include "version.h"
#include "vcut.h"


const float pi = 3.1415927;

layout (std430, binding=vcutLonLat_idx) buffer vcutLonLat
{
  float lonlat[];
};

layout (std430, binding=vcutValues_idx) buffer vcutValues
{
  float values[];
};

layout (std430, binding=vcutHeight_idx) buffer vcutHeight
{
  float height[];
};

uniform mat4 MVP;
uniform int Nx, Nz;
uniform float valmin, valmax;
uniform float dz = 0.05f;
uniform bool luniformz = true;
uniform bool lconstantz = false;

out float val;
out float skip;

void main()
{
  int i = gl_InstanceID % (Nx - 1);
  int j = gl_VertexID;
  int k = gl_InstanceID / (Nx - 1);

  skip = 0.0f;
  val = 0.0f;

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

  float z;
 
  if (luniformz)
    z = (k + dk) * dz;
  else if (lconstantz)
    z = height[k+dk];
  else
    z = height[ixz];


  val = (values[ixz] - valmin) / (valmax - valmin);

  float lon = lonlat[2*(i+di)+0];
  float lat = lonlat[2*(i+di)+1];

  if (lat > pi)
    {
      skip = 1.0f;
      return;
    }

  float coslon = cos (lon), sinlon = sin (lon);
  float coslat = cos (lat), sinlat = sin (lat);

  vec3 vertexPos = vec3 (coslon * coslat, sinlon * coslat, sinlat);

  vertexPos = (1 + z) * vertexPos;

  gl_Position = MVP * vec4 (vertexPos, 1);

}
