
#include "version.h"
#include "palette.h"

in 
#include "ISOFILL1_GS.h"

out vec4 color;

void main ()
{
  color = rgba_[int (isofill1_gs.fragmentColInd)];
}
