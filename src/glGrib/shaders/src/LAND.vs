
#include "version.h"

layout(location = 0) in vec2 vertexLonLat;

uniform mat4 MVP;

#include "schmidt.h"
#include "projection.h"
#include "scale.h"

uniform float posmax = 0.97;

out 
#include "LAND_VS.h"

void main()
{
  vec3 vertexPos = posFromLonLat (vertexLonLat);
  vec3 normedPos = compNormedPos (vertexPos);
  vec3 pos = compProjedPos (vertexPos, normedPos);
  pos = scalePosition (pos, normedPos, scale0);

  if ((proj == LATLON) || (proj == MERCATOR))
  if ((pos.y < -posmax) || (+posmax < pos.y))
    {
      pos.x = -0.1;
    }

  land_vs.fragmentPos = normedPos;

  gl_Position =  MVP * vec4 (pos, 1.);
}
