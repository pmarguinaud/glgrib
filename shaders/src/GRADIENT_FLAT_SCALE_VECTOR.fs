
#version 330 core

in float alpha;
in float fragmentVal;
in vec3 fragmentPos;

out vec4 color;

uniform vec4 color0;
uniform float fragmentValFlat = 0.0f;

#include "unpack.h"
#include "enlight.h"

void main ()
{

  if (alpha == 0.)
    discard;

  if (color0 != vec4 (0.0f, 0.0f, 0.0f, 0.0f))
    {
      color = color0;
    }
  else
    {
      color = enlightFragment (fragmentPos, fragmentVal, 0.0f);
    }

}

