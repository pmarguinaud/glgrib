
#version 330 core

//in vec3 fragmentPos;
//in float fragmentColInd;
in float fragmentColInd_;
flat in float fragmentColInd_Flat;
out vec4 color;

uniform vec4 RGBA0[256];

void main ()
{
  if (fragmentColInd_ != fragmentColInd_Flat)
    discard;
  color = RGBA0[int (fragmentColInd_)];
}
