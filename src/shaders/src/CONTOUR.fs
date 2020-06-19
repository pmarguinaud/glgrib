
#include "version.h"

in
#include "CONTOUR_VS.h"
in float alpha;
in float dist;
out vec4 color;

uniform vec4 color0;
uniform int N = 0;
uniform bool pattern[256];
uniform float length;
uniform bool dash;

void main ()
{
  if (contour_vs.alpha < 1.0)
    discard;

  if(! dash)
    {
      color = color0;
    }
  else
    {
      float r = mod (contour_vs.dist / length, 1.0f);
      int k = int (N * r);

      color.r = color0.r;
      color.g = color0.g;
      color.b = color0.b;

      if (pattern[k])
        color.a = color0.a;
      else
        color.a = 0.;
   }
}

