
#include "version.h"

in float alpha;
in float islatcst;
in vec3 fragmentPos;
out vec4 color;


uniform float dlat = 10.0f;
uniform float dlon = 10.0f;
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
      int lonm = int (mod (int (lon / dlon), 2));
      color = lonm == 0 ? colorb : colorw;
    }
  else
    {
      int latm = int (mod (int (lat / dlat), 2));
      color = latm == 0 ? colorb : colorw;
    }


}

