
#include "version.h"

in
#include "GRID_VS.h"

out vec4 color;

uniform vec4 color0; 
uniform int frag_resolution = 10;
uniform int frag_nn = 100;
uniform int frag_do_lat = 0;
uniform float dash_length = 4;

const float pi = 3.1415926;
const float rad2deg = 180.0 / pi;

void main()
{
  int k = gl_PrimitiveID;

  float lat = asin (grid_vs.vertexPos.z);
  float lon = mod (atan (grid_vs.vertexPos.y, grid_vs.vertexPos.x), 2 * pi);

  if (frag_do_lat == 0)
    {
      if ((k + 1) % (frag_nn + 1) == 0)
        discard;
      if (grid_vs.dashed > 0.0f)
        {
          if (mod (cos (lat) * lon * rad2deg, dash_length) < dash_length / 2.0)
            discard;
        }
    }
  else
    {
      if ((k + 1) % (frag_nn / 2 + 1) == 0)
        discard;
      if (grid_vs.dashed > 0.0f)
        {
          if (mod (lat * rad2deg, dash_length) < dash_length / 2.0)
            discard;
        }
    }


  color = color0;

}
