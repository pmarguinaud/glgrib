

#include "version.h"

out vec2 fragmentTex;

uniform mat4 MVP;
uniform float x0;
uniform float y0;
uniform float x1;
uniform float y1;

void main ()
{
  vec2 pos2;

  if (gl_VertexID == 0)
    pos2 = vec2 (0.0f, 0.0f);
  else if (gl_VertexID == 1)
    pos2 = vec2 (1.0f, 0.0f);
  else if (gl_VertexID == 2)
    pos2 = vec2 (1.0f, 1.0f);
  else if (gl_VertexID == 3)
    pos2 = vec2 (0.0f, 1.0f);

  fragmentTex = pos2;

  pos2 = vec2 (x0, y0) + vec2 (x1 - x0, y1 - y0) * pos2;

  gl_Position =  MVP * vec4 (0., pos2.x, pos2.y, 1.);

}


