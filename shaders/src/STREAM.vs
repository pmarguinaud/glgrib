
#version 330 core

layout(location = 0) in vec2 vertexLonLat0;
layout(location = 1) in vec2 vertexLonLat1;
layout(location = 2) in vec2 vertexLonLat2;
layout(location = 3) in float norm0;
layout(location = 4) in float norm1;
layout(location = 5) in float dist0;
layout(location = 6) in float dist1;


out float alpha;
out float dist;
out float norm;


uniform mat4 MVP;
uniform float normmax;

#include "projection.h"
#include "scale.h"
#include "linevertex.h"

void main ()
{
  float c0 = max (0.5f, 2.0f * norm0 / normmax);

  vec3 pos;

  norm = alpha = min (norm0, norm1);

  getLineVertex (vertexLonLat0, vertexLonLat1, vertexLonLat2,
                 0.0f, 0.0f, c0,
                 false, dist0, dist1, pos, dist, alpha);

  gl_Position =  MVP * vec4 (pos, 1);
}

