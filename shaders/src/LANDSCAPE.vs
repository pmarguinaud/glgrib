
#include "version.h"

layout (location = 0) in vec2 vertexLonLat;
layout (location = 1) in float vertexHeight;

out 
#include "LANDSCAPE_VS.h"

uniform mat4 MVP;

#include "schmidt.h"
#include "projection.h"
#include "scale.h"

void main()
{
  vec3 vertexPos = posFromLonLat (vertexLonLat);
  vec3 normedPos = compNormedPos (vertexPos);
  vec3 pos = compProjedPos (vertexPos, normedPos);
  pos = scalePosition (pos, normedPos, scale0);
  pos = pos * (1.0f + vertexHeight);
  gl_Position =  MVP * vec4 (pos, 1.);
  landscape_vs.fragmentPos = normedPos;
}
