#include "version.h"

in
#include "SCALAR_VS.h"

out vec4 color;

#include "unpack.h"
#include "enlight.h"

uniform vec4 selector_color;

void main ()
{
  if (abs (scalar_vs.fragmentMPI - scalar_vs.fragmentMPIFlat) > 0.0001)
    discard;
  color = enlightFragment (scalar_vs.fragmentPos, scalar_vs.fragmentVal, 
                           scalar_vs.missingFlag, scalar_vs.fragmentValFlat);
  if (scalar_vs.selected > 0)
    {
      color.rgb = (1. - scalar_vs.selected) * color.rgb + scalar_vs.selected * selector_color.rgb;
    }
}
