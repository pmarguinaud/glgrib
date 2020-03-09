
#include "version.h"

in 
#include "VECTOR_VS.h"

out vec4 color;

uniform vec4 color0;

#include "unpack.h"
#include "enlight.h"

void main ()
{

  if (vector_vs.alpha == 0.)
    discard;

  if (color0 != vec4 (0.0f, 0.0f, 0.0f, 0.0f))
    {
      color = color0;
    }
  else
    {
      color = enlightFragment (vector_vs.fragmentPos, vector_vs.fragmentVal, 0.0f, 0.0f);
    }

}

