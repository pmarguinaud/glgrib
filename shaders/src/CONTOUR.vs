
#include "version.h"

layout(location = 0) in vec2 vertexLonLat0;
layout(location = 1) in vec2 vertexLonLat1;
layout(location = 2) in vec2 vertexLonLat2;
layout(location = 3) in float vertexHeight0;
layout(location = 4) in float vertexHeight1;
layout(location = 5) in float dist0;
layout(location = 6) in float dist1;

out
#include "CONTOUR_VS.h"


uniform mat4 MVP;

#include "schmidt.h"
#include "projection.h"
#include "scale.h"
#include "linevertex.h"

void main ()
{
  float c0 = 1.0f;

  vec3 pos;

  contour_vs.alpha = 1.0f;

  getLineVertex (vertexLonLat0, vertexLonLat1, vertexLonLat2,
                 vertexHeight0, vertexHeight1, c0,
                 true, dist0, dist1, pos, contour_vs.dist, contour_vs.alpha);

  gl_Position =  MVP * vec4 (pos, 1);
}
