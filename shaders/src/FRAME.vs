
#version 330 core

layout(location = 0) in vec2 vertexLonLat0;
layout(location = 1) in vec2 vertexLonLat1;
layout(location = 2) in vec2 vertexLonLat2;


out float alpha;
out vec3 fragmentPos;
out float islatcst;


uniform mat4 MVP;

#include "projection.h"
#include "scale.h"

uniform bool do_alpha = false;
uniform float posmax = 0.97;
uniform float width = 0.005;
uniform float scale = 1.0f;

void main ()
{
  vec3 vertexPos0 = posFromLonLat (vertexLonLat0);
  vec3 vertexPos1 = posFromLonLat (vertexLonLat1);
  vec3 vertexPos2 = posFromLonLat (vertexLonLat2);

  islatcst = vertexLonLat0.y == vertexLonLat1.y ? 1.0 : 0.0f;

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

  fragmentPos = vertexPos;

  vertexPos = scale * vertexPos;

  vec3 p = normalize (vertexPos);
  vec3 n0 = cross (t0, p);
  vec3 n1 = cross (t1, p);

  float c = width / scalingFactor (p);

  if ((gl_VertexID >= 4) && (dot (cross (n0, n1), vertexPos) < 0.))
    c = 0.0;

  if (gl_VertexID == 2)
    vertexPos = vertexPos + c * n0;
  if (gl_VertexID == 3)
    vertexPos = vertexPos + c * n0;
  if (gl_VertexID == 4)
    vertexPos = vertexPos + c * normalize (n0 + n1);
  if (gl_VertexID == 5)
    vertexPos = vertexPos + c * n1;

  vec3 normedPos = compNormedPos (vertexPos);
  vec3 pos = compProjedPos (vertexPos, normedPos);

  alpha = min (min (1.0f, length (vertexPos0)), min (1.0f, length (vertexPos1)));

  if (proj == XYZ)
    {
      pos = scalePosition (pos, normedPos, scale0);
    }
  else
    {
      if ((proj == LATLON) || (proj == MERCATOR))
      if ((pos.y < -posmax) || (+posmax < pos.y))
        {
          pos.x = -0.1;
          if (do_alpha)
            alpha = 0.0;
	}
      if (proj == LATLON)
      if ((pos.z > +0.49) || (pos.z < -0.49))
        alpha = 0.0;

      if (proj == POLAR_SOUTH)
        pos.x = pos.x - 0.005;
      else
        pos.x = pos.x + 0.005;
    }

  gl_Position =  MVP * vec4 (pos, 1);

}
