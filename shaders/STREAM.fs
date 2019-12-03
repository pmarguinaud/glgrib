
#version 330 core

in float alpha;
in float dist;
in float norm;
out vec4 color;

uniform vec4 RGBA0[256];
uniform float palmin;
uniform float palmax;
uniform float valmin;
uniform float valmax;

void main ()
{
  if (alpha == 0.0f)
    discard;

  vec3 grey = vec3 (0.3f, 0.3f, 0.3f);
  vec3 green= vec3 (0.0f, 1.0f, 0.0f);

  float n = norm / valmax;
  int k = min (255, 1 + int (n * 254.0));
  color = RGBA0[k];

}

