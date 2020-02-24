
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
uniform bool motion = false;

void main ()
{
  if (alpha == 0.0f)
    discard;

  int k;

  if (motion)
    {
      float y = (1 + sin (3000 * dist - 10.0 * timea)) / 2;
      k = int (255 * min (1, max (0, y)));
    }
  else
    {
      float n = norm / valmax;
      k = min (255, 1 + int (n * 254.0));
    }
  
  color = RGBA0[k];

}

