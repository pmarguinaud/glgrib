
#version 330 core

layout(location = 0) in vec2 vertexLonLat0;
layout(location = 1) in vec2 vertexLonLat1;
layout(location = 2) in vec2 vertexLonLat2;
layout(location = 3) in float vertexHeight0;
layout(location = 4) in float vertexHeight1;
layout(location = 5) in float dist0;
layout(location = 6) in float dist1;


out float alpha;
out float dist;


uniform mat4 MVP;

#include "projection.h"
#include "scale.h"
#include "linevertex.h"

void main ()
{
  float c0 = 1.0f;

  vec3 pos;

  alpha = 1.0f;

  getLineVertex (vertexLonLat0, vertexLonLat1, vertexLonLat2,
                 vertexHeight0, vertexHeight1, c0,
                 true, dist0, dist1, pos, dist, alpha);

  gl_Position =  MVP * vec4 (pos, 1);
}
