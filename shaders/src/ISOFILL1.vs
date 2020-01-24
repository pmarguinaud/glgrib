
#version 330 core

layout(location = 0) in vec2  vertexLonLat;
layout(location = 1) in float vertexColInd;

out vec3 fragmentPos;
out float fragmentColInd;
flat out float fragmentColIndFlat;

uniform mat4 MVP;

#include "projection.h"
#include "scale.h"

void main ()
{
  fragmentColIndFlat = vertexColInd;
  fragmentColInd     = vertexColInd;

  vec3 vertexPos = posFromLonLat (vertexLonLat);
  vec3 normedPos = compNormedPos (vertexPos);
  vec3 pos = compProjedPos (vertexPos, normedPos);
  pos = scalePosition (pos, normedPos, scale0);

  gl_Position =  MVP * vec4 (pos, 1.);

  fragmentPos = normedPos;

}
