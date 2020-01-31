
#version 330 core

in vec3 fragmentPos;
in float fragmentColInd;
out vec4 color;

uniform vec4 RGBA0[256];

void main ()
{
  color = RGBA0[int (fragmentColInd)];
}
