
#version 330 core

layout(location = 0) in vec2  vertexLonLat;
layout(location = 1) in float vertexVal;
layout(location = 2) in float vertexHeight;

out float fragmentVal;
out vec3 fragmentPos;
out float missingFlag;
flat out float fragmentValFlat;

uniform mat4 MVP;
uniform float height_scale = 0.05;

#include "projection.h"
#include "scale.h"

const float rad2deg = 180.0 / pi;
const float deg2rad = pi / 180.0;

void main ()
{
  fragmentValFlat = vertexVal;

  vec3 vertexPos = posFromLonLat (vertexLonLat);
  vec3 normedPos = compNormedPos (vertexPos);
  vec3 pos = compProjedPos (vertexPos, normedPos);
  pos = scalePosition (pos, normedPos, scale0);

  if (proj_vs == XYZ)
    pos = pos * (1.0f + height_scale * vertexHeight);
    
  gl_Position =  MVP * vec4 (pos, 1.);

  fragmentVal = vertexVal;
  fragmentPos = normedPos;
  missingFlag = vertexVal == 0 ? 1. : 0.;

}
