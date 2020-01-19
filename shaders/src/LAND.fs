
#version 330 core

#include "projection_kind.h"

in vec3 fragmentPos;
out vec4 color;

uniform vec4 color0 = vec4 (0.0f, 1.0f, 0.0f, 1.0f);
uniform bool debug = false;
uniform float lon0fs = 180.0f;
uniform int proj_fs = 0;

const float pi = 3.1415926;
const float deg2rad = pi / 180.0;
const float rad2deg = 180.0 / pi;



void main()
{
  if ((proj_fs == LATLON) || (proj_fs == MERCATOR))
    {
      float lon = rad2deg * atan (fragmentPos.y, fragmentPos.x);  
      float dlon = 6.0f;
     
      while (lon > lon0fs)
        lon = lon - 360.0;
      while (lon < lon0fs)
        lon = lon + 360.0;
     
      if (abs (lon - lon0fs) < dlon)
        discard;
      if (abs (lon - 360.0 - lon0fs) < dlon)
        discard;
      if (abs (lon + 360.0 - lon0fs) < dlon)
        discard;
    }

  if (debug)
    {
      int k = gl_PrimitiveID % 8;
      
      vec3 colors[8];
      colors[0] = vec3 (1.0f, 0.0f, 0.0f);
      colors[1] = vec3 (0.0f, 1.0f, 0.0f);
      colors[2] = vec3 (0.0f, 0.0f, 1.0f);
      colors[3] = vec3 (0.9f, 0.9f, 0.6f);
      colors[4] = vec3 (0.0f, 0.1f, 1.0f);
      colors[5] = vec3 (1.0f, 0.0f, 1.0f);
      colors[6] = vec3 (0.5f, 0.5f, 0.5f);
      colors[7] = vec3 (0.5f, 1.0f, 0.5f);
      
      color.rgb = colors[k];
      color.a   = 1.;
    }
  else
    {
      color = color0;
    }

}
