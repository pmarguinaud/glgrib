#include "version.h"

out vec4 color;

in
#include "SCALAR_VS.h"

#include "unpack.h"
#include "enlight.h"


void main()
{
  if (scalar_vs.missingFlag > 0.0f)
    {
      discard;
    }
  else
    {
      color = enlightFragment (scalar_vs.fragmentPos, scalar_vs.fragmentVal, 
                               scalar_vs.missingFlag, scalar_vs.fragmentValFlat);
    }
}
