
#include "version.h"

in float val;
out vec4 color;

void main()
{
  color.rgb = vec3 (1.0f - val, val, 0.0f);
  color.a   = 1.0f;
}
