
#version 330 core

in vec3 fragmentPos;
in float fragmentColInd;
flat in float fragmentColIndFlat;
out vec4 color;

uniform vec4 RGBA0[256];

void main ()
{
  if (fragmentColInd != fragmentColIndFlat)
    discard;
  color = RGBA0[int (fragmentColInd)];
}
