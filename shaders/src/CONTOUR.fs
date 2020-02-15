
#include "version.h"

in float alpha;
in float dist;
out vec4 color;

uniform vec3 color0;
uniform int N = 0;
uniform bool pattern[256];
uniform float length;
uniform bool dash;

void main ()
{
  if (alpha < 1.)
    discard;
  if(! dash)
    {
      color.r = color0.r;
      color.g = color0.g;
      color.b = color0.b;
      color.a = 1.;
    }
  else
    {
      float r = mod (dist / length, 1.0f);
      int k = int (N * r);

      color.r = color0.r;
      color.g = color0.g;
      color.b = color0.b;

      if (pattern[k])
        color.a = 1.;
      else
        color.a = 0.;

if(false){
      if (r > 0.5f)
        {
          color.r = 1.;
          color.g = 0.;
          color.b = 0.;
        }
      else
        {
          color.r = 0.;
          color.g = 1.;
          color.b = 0.;
        }
      color.a = 1.;
}
      
  }
}

