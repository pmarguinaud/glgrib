#include "version.h"
#include "palette.h"

in float pointVal;
in vec3 centerVec;
flat in float pointRad;
out vec4 color;

uniform vec4 color0;
uniform float palmin;
uniform float palmax;

uniform bool lcolor0;


void main()
{
  if (lcolor0)
    {
      color.r = color0.r;
      color.g = color0.g;
      color.b = color0.b;
      color.a = color0.a;
    }
  else
    {
      int pal = max (1, min (int (1 + 254 * (pointVal - palmin) / (palmax - palmin)), 255));
      color.r = rgba_[pal].r;
      color.g = rgba_[pal].g;
      color.b = rgba_[pal].b;
      color.a = rgba_[pal].a;
    }
  if (color.r == 0. && color.g == 0. 
   && color.b == 0. && color.a == 0.)
    discard;

  if(false){
  // Round shape
  if (length (centerVec) > pointRad)
    discard;
  }else if(false){
  // Diamond shape
  if (abs (centerVec.x) + abs (centerVec.y) + abs (centerVec.z) > pointRad)
    discard;
  }

}
