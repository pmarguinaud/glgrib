
#include "version.h"

in float fragmentVal;
in float fragmentMPI;
in vec3 fragmentPos;
in float missingFlag;
flat in float fragmentValFlat;
flat in float fragmentMPIFlat;

out vec4 color;

#include "unpack.h"
#include "enlight.h"

void main ()
{
  if (fragmentMPI != fragmentMPIFlat)
    discard;
  color = enlightFragment (fragmentPos, fragmentVal, missingFlag);
}
