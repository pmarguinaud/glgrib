
#version 330 core

in float fragmentVal;
in vec3 fragmentPos;
in float missingFlag;
flat in float fragmentValFlat;
out vec4 color;

#include "unpack.h"
#include "enlight.h"

void main ()
{
  color = enlightFragment (fragmentPos, fragmentVal, missingFlag);
}
