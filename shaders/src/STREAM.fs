
#include "version.h"

in float alpha;
in float dist;
in float norm;
out vec4 color;

uniform vec4 RGBA0[256];
uniform float palmin;
uniform float palmax;
uniform float valmin;
uniform float valmax;
uniform float timea;
uniform float width; // From linevertex.h
uniform bool motion = true;

void main ()
{
  if (alpha == 0.0f)
    discard;

  float y = (1 + sin (3000 * dist - 10.0 * timea)) / 2;

  float n = norm / valmax;
  int k = min (255, 1 + int (n * 254.0));
  
  vec4 r = vec4 (1.0, 0.0, 0.0, 1.0);
  vec4 b = vec4 (0.0, 0.0, 1.0, 1.0);
  
//color =  y * RGBA0[k];

  color = y * r + (1 - y) * b;

}

