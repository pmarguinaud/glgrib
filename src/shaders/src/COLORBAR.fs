#include "version.h"
#include "palette.h"

flat in int rank;

out vec4 color;

uniform int rank2rgba[256];

void main ()
{
  color = rgba_[rank2rgba[rank]];
}

