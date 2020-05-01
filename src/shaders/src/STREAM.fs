
#include "version.h"
#include "palette.h"

in 
#include "STREAM_VS.h"

out vec4 color;

uniform float palmin;
uniform float palmax;
uniform float valmin;
uniform float valmax;
uniform float timea;
uniform float width; // From linevertex.h
uniform bool motion = false;
uniform float accelt = 10000.0f;
uniform float nwaves = 10.0f;       // 10 waves over 1000km

void main ()
{
  if (stream_vs.alpha == 0.0f)
    discard;

  int k;

  if (motion)
    {
      const float ra = 6000000.0f;
      const float distref = 1000000.0f; // = 1000 km
      float distrefovervalmax = distref / valmax;
      float tscale = distrefovervalmax / nwaves;
      float y = (1 + sin ((ra * stream_vs.dist - timea * accelt) / tscale)) / 2;
      k = 1 + int (254 * min (1, max (0, y)));
    }
  else
    {
      float n = stream_vs.norm / valmax;
      k = min (255, 1 + int (n * 254.0));
    }
  
  color = rgba_[k];

}

