
#include "version.h"

layout (location = 0) in vec3 xya;

uniform mat4 MVP;
uniform float N;
uniform float S;
uniform float W;
uniform float E;
uniform float length = 0.05f;
uniform float width  = 0.01f;
uniform int kind = 0;

void main()
{
  float x = xya.x, y = xya.y, a = xya.z;
  vec2 pos2 = vec2 (x + 0.0f, y + 0.0f);

  if (kind == 0)
    {
      if ((gl_VertexID == 1) || (gl_VertexID == 2))
        {
          int kx = 0, ky = 0;
          if (a == N) kx = -1;
          if (a == S) kx = +1;
          if (a == E) ky = +1;
          if (a == W) ky = -1;
          pos2 = pos2 + vec2 (+kx * width / 2, +ky * width / 2);
        }
     
      if ((gl_VertexID == 0) || (gl_VertexID == 3))
        {
          int kx = 0, ky = 0;
          if (a == N) kx = -1;
          if (a == S) kx = +1;
          if (a == E) ky = +1;
          if (a == W) ky = -1;
          pos2 = pos2 + vec2 (-kx * width / 2, -ky * width / 2);
        }
     
      if ((gl_VertexID == 2) || (gl_VertexID == 3))
        {
          int kx = 0, ky = 0;
          if (a == N) ky = -1;
          if (a == S) ky = +1;
          if (a == E) kx = -1;
          if (a == W) kx = +1;
          pos2 = pos2 + vec2 (kx * length, ky * length);
        }
    }
  else if (kind == 1)
    {
      if (gl_VertexID == 0)
        {
          int kx = 0, ky = 0;
          if (a == N) kx = -1;
          if (a == S) kx = +1;
          if (a == E) ky = +1;
          if (a == W) ky = -1;
          pos2 = pos2 + vec2 (-kx * width / 2, -ky * width / 2);
        }
      else if (gl_VertexID == 1)
        {
          int kx = 0, ky = 0;
          if (a == N) kx = -1;
          if (a == S) kx = +1;
          if (a == E) ky = +1;
          if (a == W) ky = -1;
          pos2 = pos2 + vec2 (+kx * width / 2, +ky * width / 2);
        }
      else if (gl_VertexID == 2)
        {
          int kx = 0, ky = 0;
          if (a == N) ky = -1;
          if (a == S) ky = +1;
          if (a == E) kx = -1;
          if (a == W) kx = +1;
          pos2 = pos2 + vec2 (kx * length, ky * length);
        }
    }

  // Behind labels
  gl_Position =  MVP * vec4 (-0.001, pos2.x, pos2.y, 1.);

}
