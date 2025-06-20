
#include "version.h"

layout(location = 0) in vec3 vertexLonLatCst0;
layout(location = 1) in vec3 vertexLonLatCst1;
layout(location = 2) in vec3 vertexLonLatCst2;


out
#include "FRAME_VS.h"


uniform mat4 MVP;

#include "schmidt.h"
#include "projection.h"
#include "scale.h"

uniform bool do_alpha = false;
uniform float posmax = 0.97;
uniform float width = 0.005;

void main ()
{
  vec3 vertexPos0 = posFromLonLat (vertexLonLatCst0.xy);
  vec3 vertexPos1 = posFromLonLat (vertexLonLatCst1.xy);
  vec3 vertexPos2 = posFromLonLat (vertexLonLatCst2.xy);

  frame_vs.islatcst = vertexLonLatCst0.z;

  vec3 vertexPos;

  vec3 t0 = normalize (vertexPos1 - vertexPos0);
  vec3 t1 = normalize (vertexPos2 - vertexPos1);

  if ((gl_VertexID == 0) || (gl_VertexID == 2))
    {
      vertexPos = vertexPos0;
    }
  else if ((gl_VertexID == 1) || (gl_VertexID == 3) || (gl_VertexID == 5) || (gl_VertexID == 4))
    {
      vertexPos = vertexPos1;  
    }

  frame_vs.fragmentPos = vertexPos;

  vec3 vertexPosA = vertexPos;
  vec3 vertexPosB = vertexPos;

  vec3 p = normalize (vertexPosB);
  vec3 n0 = cross (t0, p);
  vec3 n1 = cross (t1, p);

  float c = width / scalingFactor (p);

  if ((gl_VertexID >= 4) && (dot (cross (n0, n1), vertexPosB) < 0.))
    c = 0.0;

  if (gl_VertexID == 2)
    vertexPosB = vertexPosB + c * n0;
  if (gl_VertexID == 3)
    vertexPosB = vertexPosB + c * n0;
  if (gl_VertexID == 4)
    vertexPosB = vertexPosB + c * normalize (n0 + n1);
  if (gl_VertexID == 5)
    vertexPosB = vertexPosB + c * n1;

  vec3 normedPosA = compNormedPos (vertexPosA);
  vec3 normedPosB = compNormedPos (vertexPosB);
  vec3 posA = compProjedPos (vertexPosA, normedPosA);
  vec3 posB = compProjedPos (vertexPosB, normedPosB);

  frame_vs.alpha = min (min (1.0f, length (vertexPos0)), min (1.0f, length (vertexPos1)));

  if (proj == XYZ)
    {
      posB = scalePosition (posB, normedPosB, scale0);
    }
  else 
    {
      if ((gl_VertexID != 0) && (gl_VertexID != 1))
        posB = posA + width * 0.3 * normalize (posB - posA);

      if ((proj == LATLON) || (proj == MERCATOR))
      if ((posB.y < -posmax) || (+posmax < posB.y))
        {
          posB.x = -0.1;
          if (do_alpha)
            frame_vs.alpha = 0.0;
	}
      if (proj == LATLON)
      if ((posB.z > +0.49) || (posB.z < -0.49))
        frame_vs.alpha = 0.0;

      if (proj == POLAR_SOUTH)
        posB.x = posB.x - 0.005;
      else
        posB.x = posB.x + 0.005;
    }

  gl_Position =  MVP * vec4 (posB, 1);

}
