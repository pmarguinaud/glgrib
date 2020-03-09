
#include "version.h"

layout(location = 0) in vec2 vertexLonLat0;
layout(location = 1) in vec2 vertexLonLat1;
layout(location = 2) in vec2 vertexLonLat2;
layout(location = 3) in float norm0;
layout(location = 4) in float norm1;
layout(location = 5) in float dist0;
layout(location = 6) in float dist1;


out
#include "STREAM_VS.h"

uniform mat4 MVP;
uniform float normmax;
uniform bool scalenorm = false;

#include "schmidt.h"
#include "projection.h"
#include "scale.h"
#include "linevertex.h"

void main ()
{
  float c0 = scalenorm ? max (0.5f, 2.0f * norm0 / normmax) : 1.0f;

  vec3 pos;

  stream_vs.norm = stream_vs.alpha = min (norm0, norm1);

  getLineVertex (vertexLonLat0, vertexLonLat1, vertexLonLat2,
                 0.0f, 0.0f, c0,
                 false, dist0, dist1, pos, stream_vs.dist, stream_vs.alpha);

  gl_Position =  MVP * vec4 (pos, 1);
}

