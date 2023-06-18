
#include "version.h"

in float val;
out vec4 color;

in
#include "SCALAR_VS.h"

uniform vec4 colormax, colormin;

#include "unpack.h"
#include "enlight.h"


void main()
{
  if (scalar_vs.missingFlag > 0.0f)
    {
      discard;
    }
  else
    {
      color = colormax * val + (1.0f - val) * colormin;
    }
}
