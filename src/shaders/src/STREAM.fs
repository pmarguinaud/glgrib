
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

  float rgba_size2 = float (rgba_size - 2);

  if (motion)
    {
      const float ra = 6000000.0f;
      const float distref = 1000000.0f; // = 1000 km
      float distrefovervalmax = distref / valmax;
      float tscale = distrefovervalmax / nwaves;
      float y = (1 + sin ((ra * stream_vs.dist - timea * accelt) / tscale)) / 2;
      k = 1 + int (rgba_size2 * min (1, max (0, y)));
    }
  else
    {
      float n = stream_vs.norm / valmax;
      k = min (rgba_size-1, 1 + int (n * rgba_size2));
    }
  
  color = rgba_[k];

}

