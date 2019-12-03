
#version 330 core

in float fragmentVal;
in vec3 fragmentPos;
in float missingFlag;
out vec4 color;

#include "enlight.h"

void main ()
{
  color = enlightFragment (fragmentPos, fragmentVal, missingFlag);
}
