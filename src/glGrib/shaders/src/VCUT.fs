
#include "version.h"

in float val;
in float skip;
out vec4 color;

in
#include "SCALAR_VS.h"

uniform vec4 colormax, colormin;

#include "unpack.h"
#include "enlight.h"


void main()
{
  if (skip > 0.0f)
    {
      discard;
    }
  else
    {
      color = colormax * val + (1.0f - val) * colormin;
    }
}
