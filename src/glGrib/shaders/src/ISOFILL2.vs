
#include "version.h"

layout(location = 0) in vec2  vertexLonLat;

out 
#include "ISOFILL2_VS.h"

uniform mat4 MVP;

#include "schmidt.h"
#include "projection.h"
#include "scale.h"

void main ()
{
  vec3 vertexPos = posFromLonLat (vertexLonLat);
  vec3 normedPos = compNormedPos (vertexPos);
  vec3 pos = compProjedPos (vertexPos, normedPos);
  pos = scalePosition (pos, normedPos, scale0);

  gl_Position =  MVP * vec4 (pos, 1.);

  isofill2_vs.fragmentPos = normedPos;

}
