
#version 440 core

layout(location = 0) in vec2  vertexLonLat;
layout(location = 1) in float vertexVal;
layout(location = 2) in float vertexHeight;
layout(location = 3) in vec3  vertexMPIView;
layout(location = 4) in float vertexMask;


out 
#include "SCALAR_VS.h"

uniform mat4 MVP;
uniform float height_scale = 0.05;
uniform float mpiview_scale = 0.0f;
uniform int frame = 0;
uniform bool useVertexMask = false;

#include "schmidt.h"
#include "projection.h"
#include "scale.h"

const float rad2deg = 180.0 / pi;
const float deg2rad = pi / 180.0;

#include "geometry.h"

void main ()
{
  vec3 vertexDisp = vec3 (0.0f, 0.0f, 0.0f);

  vec2 vertexLonLat_ = getVertexLonLat (gl_VertexID);

  if (mpiview_scale > 0.0f)
    {
      float lon = vertexMPIView.y, lat = vertexMPIView.z;
      vertexDisp = posFromLonLat (vec2 (lon, lat));
    }

  scalar_vs.fragmentValFlat = vertexVal;
  scalar_vs.fragmentMPIFlat = vertexMPIView.x;

  vec3 vertexPos = posFromLonLat (vertexLonLat_);
  vec3 normedPos = compNormedPos (vertexPos);
  vec3 pos = compProjedPos (vertexPos, normedPos);
  pos = scalePosition (pos, normedPos, scale0);

  if (proj == XYZ)
    {
      pos = pos * (1.0f + height_scale * vertexHeight);
      if (mpiview_scale > 0.0f)
        pos = pos + mpiview_scale * vertexDisp;
    }
    
  gl_Position =  MVP * vec4 (pos, 1.);

  scalar_vs.fragmentVal = vertexVal;
  scalar_vs.fragmentMPI = vertexMPIView.x;
  scalar_vs.fragmentPos = normedPos;
  scalar_vs.missingFlag = vertexVal == 0 ? 1. : 0.;

  if ((vertexMask < frame) && useVertexMask)
    scalar_vs.missingFlag = 1.;

}
