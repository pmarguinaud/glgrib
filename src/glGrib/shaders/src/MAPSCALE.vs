#include "version.h"

uniform mat4 MVP;
uniform float xmin = 0.10;
uniform float xmax = 0.20; 
uniform float ymin = 0.10; 
uniform float ymax = 0.12;

flat out int rank;

void main()
{
  float x, y;
  
  rank = int (mod (gl_VertexID / 4, 2));

  int ix = int (gl_VertexID / 4);

  int k = int (mod (gl_VertexID, 4));
  
  int iy = int (mod (gl_VertexID, 2));

  if (iy == 0)
    {
      if (k == 0)
        ix = ix + 1;
      else if (k == 3)
        ix = ix + 1;
    }
  else
    {
      if (k == 1)
        ix = ix + 1;
      else if (k == 2)
        ix = ix + 1;
    }

  x = xmin + ix * (xmax - xmin) / 4.0;
  y = ymin + iy * (ymax - ymin) / 1.0;

  vec2 vertexPos = vec2 (x, y);
  gl_Position =  MVP * vec4 (0., vertexPos.x, vertexPos.y, 1.);
}

