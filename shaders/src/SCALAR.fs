
#include "version.h"

in
#include "SCALAR_VS.h"

out vec4 color;

#include "unpack.h"
#include "enlight.h"

void main ()
{
  if (scalar_vs.fragmentMPI != scalar_vs.fragmentMPIFlat)
    discard;
  color = enlightFragment (scalar_vs.fragmentPos, scalar_vs.fragmentVal, 
                           scalar_vs.missingFlag, scalar_vs.fragmentValFlat);
}
