#include "version.h"
#include "vertical.h"


layout (std430, binding=verticalLonLat_idx) buffer verticalLonLat
{
  float lonlat[];
};

layout (std430, binding=verticalValues_idx) buffer verticalValues
{
  float values[];
};

layout (std430, binding=verticalHeight_idx) buffer verticalHeight
{
  float height[];
};

#include "schmidt.h"
#include "projection.h"
#include "scale.h"

out 
#include "SCALAR_VS.h"

uniform mat4 MVP;
uniform int Nx, Nz;
uniform float dz = 0.05f;
uniform bool luniformz = true;
uniform bool lconstantz = false;

void main()
{
  int i = gl_InstanceID % (Nx - 1);
  int j = gl_VertexID;
  int k = gl_InstanceID / (Nx - 1);

  float val = 0.0f;

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


  val = values[ixz];

  scalar_vs.fragmentValFlat = val;
  scalar_vs.fragmentMPIFlat = 0.0f;

  float lon = lonlat[2*(i+di)+0];
  float lat = lonlat[2*(i+di)+1];


  if (lat > pi)
    {
      scalar_vs.missingFlag = 1.0f;
    }
  else
    {
      vec3 vertexPos = posFromLonLat (vec2 (lon, lat));
      vec3 normedPos = compNormedPos (vertexPos);
      vec3 pos = compProjedPos (vertexPos, normedPos);
      pos = scalePosition (pos, normedPos, scale0 * (1 + z));
      scalar_vs.missingFlag = val == 0. ? 1. : 0.;
      scalar_vs.fragmentPos = normedPos;
      scalar_vs.fragmentVal = val;
      scalar_vs.fragmentMPI = 0.0f;
      gl_Position = MVP * vec4 (pos, 1);
    }

}
