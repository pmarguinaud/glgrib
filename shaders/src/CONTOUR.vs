
#version 330 core

layout(location = 0) in vec2 vertexLonLat0;
layout(location = 1) in vec2 vertexLonLat1;
layout(location = 2) in vec2 vertexLonLat2;
layout(location = 3) in float vertexHeight0;
layout(location = 4) in float vertexHeight1;
layout(location = 5) in float dist0;
layout(location = 6) in float dist1;


out float alpha;
out float dist;


uniform mat4 MVP;

#include "projection.h"
#include "scale.h"

uniform bool do_alpha = false;
uniform float posmax = 0.97;
uniform float width = 0.005;

uniform float height_scale = 0.05;

void main ()
{
  vec3 vertexPos0 = posFromLonLat (vertexLonLat0);
  vec3 vertexPos1 = posFromLonLat (vertexLonLat1);
  vec3 vertexPos2 = posFromLonLat (vertexLonLat2);

  vec3 vertexPos;
  float height;

  vec3 t0 = normalize (vertexPos1 - vertexPos0);
  vec3 t1 = normalize (vertexPos2 - vertexPos1);


  if ((gl_VertexID == 0) || (gl_VertexID == 2))
    {
      vertexPos = vertexPos0;
      height    = vertexHeight0;
    }
  else if ((gl_VertexID == 1) || (gl_VertexID == 3) || (gl_VertexID == 5) || (gl_VertexID == 4))
    {
      vertexPos = vertexPos1;  
      height    = vertexHeight1;
    }

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
      pos = pos * (1.0f + height_scale * height);
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

  if ((gl_VertexID == 0) || (gl_VertexID == 2))
    {
      dist = dist0;
    }
  else
    {
      dist = dist1;
    }



}
