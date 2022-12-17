
#include "version.h"

in
#include "FRAME_VS.h"


out vec4 color;


uniform float dlat = 10.0f;
uniform float dlon = 10.0f;
uniform vec4 colorb = vec4 (0.3f, 0.3f, 0.3f, 1.0f);
uniform vec4 colorw = vec4 (1.0f, 1.0f, 1.0f, 1.0f);

void main ()
{
  const float pi = 3.1415926;
  const float rad2deg = 180.0 / pi;

  float lon = 360.0 + rad2deg * atan (frame_vs.fragmentPos.y, frame_vs.fragmentPos.x);
  float lat =  90.0 + rad2deg * asin (frame_vs.fragmentPos.z);

  if (abs (frame_vs.alpha - 1.) > 0.0001)
    discard;

  if (frame_vs.islatcst > 0.0f)
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

