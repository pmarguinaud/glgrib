
#include "version.h"

in 
#include "MONO_VS.h"

out vec4 color;

uniform vec4 color0; 

void main()
{
  color.r = color0.r;
  color.g = color0.g;
  color.b = color0.b;
  color.a = mono_vs.alpha;
}
