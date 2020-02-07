
#version 430 core

layout(location = 0) in vec2  vertexLonLat;
layout(location = 1) in float vertexVal;
layout(location = 2) in float vertexHeight;
layout(location = 3) in vec3  vertexMPIView;

out float fragmentVal;
out float fragmentMPI;
out vec3 fragmentPos;
out float missingFlag;
flat out float fragmentValFlat;
flat out float fragmentMPIFlat;

uniform mat4 MVP;
uniform float height_scale = 0.05;
uniform float mpiview_scale = 0.0f;

#include "projection.h"
#include "scale.h"

const float rad2deg = 180.0 / pi;
const float deg2rad = pi / 180.0;

layout (std430, binding=2) buffer layoutName
{
  int data_SSBO[];
};

void main ()
{
  vec3 vertexDisp = vec3 (0.0f, 0.0f, 0.0f);

  bool tt = true;
 if ((data_SSBO[0] != 0) || (data_SSBO[1] != 1)
  || (data_SSBO[2] != 2) || (data_SSBO[3] != 3))
    tt = false;

  if (mpiview_scale > 0.0f)
    {
      float lon = vertexMPIView.y, lat = vertexMPIView.z;
      float coslon = cos (lon), sinlon = sin (lon);
      float coslat = cos (lat), sinlat = sin (lat);
      vertexDisp = vec3 (coslon * coslat, sinlon * coslat, sinlat);
    }

  fragmentValFlat = vertexVal;
  fragmentMPIFlat = vertexMPIView.x;

  vec3 vertexPos = posFromLonLat (vertexLonLat);
  vec3 normedPos = compNormedPos (vertexPos);
  vec3 pos = compProjedPos (vertexPos, normedPos);
  pos = scalePosition (pos, normedPos, scale0);

  if (! tt)
    pos = pos * 0.5;

  if (proj_vs == XYZ)
    {
      pos = pos * (1.0f + height_scale * vertexHeight);
      if (mpiview_scale > 0.0f)
        pos = pos + mpiview_scale * vertexDisp;
    }
    
  gl_Position =  MVP * vec4 (pos, 1.);

  fragmentVal = vertexVal;
  fragmentMPI = vertexMPIView.x;
  fragmentPos = normedPos;
  missingFlag = vertexVal == 0 ? 1. : 0.;

}
