
#include "version.h"

in float val;
out vec4 color;

uniform vec4 colormax, colormin;

void main()
{
  color = colormax * val + (1.0f - val) * colormin;
}
