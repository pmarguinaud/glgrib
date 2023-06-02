
#include "version.h"

in float val;
in float skip;
out vec4 color;


uniform vec4 colormax, colormin;

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
