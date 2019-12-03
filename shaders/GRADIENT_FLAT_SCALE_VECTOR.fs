
#version 330 core

in float alpha;
out vec4 color;

uniform vec3 color0;

void main ()
{
  if (alpha == 0.)
    discard;
  color.r = color0.r;
  color.g = color0.g;
  color.b = color0.b;
  color.a = 1.;
}

