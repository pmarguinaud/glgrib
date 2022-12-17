
#include "version.h"

in float fragmentVal;
in vec3 fragmentPos;
in float missingFlag;
in float pointVal;
in vec3 centerVec;
flat in float pointRad;
flat in float fragmentValFlat;
out vec4 color;

#include "unpack.h"
#include "enlight.h"

void main ()
{
//if (length (centerVec) > pointRad)
//  discard;
//if (abs (centerVec.x) + abs (centerVec.y) + abs (centerVec.z) > pointRad)
//  discard;
  color = enlightFragment (fragmentPos, fragmentVal, missingFlag, fragmentValFlat);
}
