#include "version.h"

uniform mat4 MVP;
uniform float xmin = 0.08;
uniform float xmax = 0.10; 
uniform float ymin = 0.05; 
uniform float ymax = 0.95;

flat out int rank;

void main()
{
  rank = gl_VertexID / 4;
  int corn = int (mod (gl_VertexID, 4));
  int ix = int (mod (corn, 2));
  int iy = corn / 2;
  float x = xmin + (xmax - xmin) * ix;
  float y = ymin + (rank + iy) * (ymax - ymin) / 255.0;
  vec2 vertexPos = vec2 (x, y);
  gl_Position =  MVP * vec4 (0., vertexPos.x, vertexPos.y, 1.);
}

