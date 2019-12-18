
#version 330 core

in float alpha;
in float islatcst;
in vec3 fragmentPos;
out vec4 color;

uniform vec3 color0;

uniform vec4 colorb = vec4 (0.3f, 0.3f, 0.3f, 1.0f);
uniform vec4 colorw = vec4 (1.0f, 1.0f, 1.0f, 1.0f);

void main ()
{
  const float pi = 3.1415926;
  const float rad2deg = 180.0 / pi;

  float lon = 360.0 + rad2deg * atan (fragmentPos.y, fragmentPos.x);
  float lat =  90.0 + rad2deg * asin (fragmentPos.z);

  if (alpha < 1.)
    discard;

  if (islatcst > 0.0f)
    {
      int lonm = int (mod (int (lon / 10.0f), 2));
      color = lonm == 0 ? colorb : colorw;
    }
  else
    {
      int latm = int (mod (int (lat / 10.0f), 2));
      color = latm == 0 ? colorb : colorw;
    }


if(false){
  color.r = color0.r;
  color.g = color0.g;
  color.b = color0.b;
  color.a = 1.;
}
}

