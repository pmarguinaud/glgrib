
#include "version.h"

in 
#include "ISOFILL1_GS.h"

out vec4 color;

uniform vec4 RGBA0[256];

void main ()
{
  color = RGBA0[int (isofill1_gs.fragmentColInd)];
}
