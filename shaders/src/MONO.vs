
#include "version.h"

layout(location = 0) in vec2 vertexLonLat;
out 
#include "MONO_VS.h"


uniform mat4 MVP;
uniform bool do_alpha = false;
uniform float posmax = 0.97;
uniform float scale = 1.005;

#include "schmidt.h"
#include "projection.h"

void main()
{
  vec3 vertexPos = posFromLonLat (vertexLonLat);
  vec3 pos;

  mono_vs.alpha = 1.0;

  if (proj == XYZ)
    {
      pos = scale * applySchmidt (vertexPos);
    }
  else
    {
      vec3 normedPos = compNormedPos (vertexPos);
      pos = compProjedPos (vertexPos, normedPos);

      if ((proj == LATLON) || (proj == MERCATOR))
      if ((pos.y < -posmax) || (+posmax < pos.y))
        {
          pos.x = -0.1;
          if (do_alpha)
            mono_vs.alpha = 0.0;
	}
      if (proj == LATLON)
      if ((pos.z > +0.49) || (pos.z < -0.49))
        mono_vs.alpha = 0.0;

      if (proj == POLAR_SOUTH)
        pos.x = pos.x - 0.005;
      else
        pos.x = pos.x + 0.005;
    }

  gl_Position =  MVP * vec4 (pos, 1.);

}
