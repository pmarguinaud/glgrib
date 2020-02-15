
#include "version.h"

in float dashed;
in vec3 vertexPos;

out vec4 color;

uniform vec3 color0; 
uniform int frag_resolution = 10;
uniform int frag_nn = 100;
uniform int frag_do_lat = 0;
uniform float dash_length = 4;

const float pi = 3.1415926;
const float rad2deg = 180.0 / pi;

void main()
{
  int k = gl_PrimitiveID;

  float lat = asin (vertexPos.z);
  float lon = mod (atan (vertexPos.y, vertexPos.x), 2 * pi);

  if (frag_do_lat == 0)
    {
      if ((k + 1) % (frag_nn + 1) == 0)
        discard;
      if (dashed > 0.0f)
        {
          if (mod (cos (lat) * lon * rad2deg, dash_length) < dash_length / 2.0)
            discard;
        }
    }
  else
    {
      if ((k + 1) % (frag_nn / 2 + 1) == 0)
        discard;
      if (dashed > 0.0f)
        {
          if (mod (lat * rad2deg, dash_length) < dash_length / 2.0)
            discard;
        }
    }


  color.r = color0.r;
  color.g = color0.g;
  color.b = color0.b;
  color.a = 1.0f;

}
