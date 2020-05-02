#include "version.h"

out vec4 color;

flat in int rank;

uniform vec4 color1 = vec4 (1.0, 1.0, 1.0, 1.0);
uniform vec4 color2 = vec4 (0.3, 0.3, 0.3, 1.0);

void main ()
{
  if (rank == 0)
    {
      color.r = color1.r;
      color.g = color1.g;
      color.b = color1.b;
      color.a = color1.a;
    }
  else
    {
      color.r = color2.r;
      color.g = color2.g;
      color.b = color2.b;
      color.a = color2.a;
    }
}

