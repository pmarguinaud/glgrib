
#version 440 core

layout(location = 0) in vec2 vertexLonLat;
layout(location = 1) in float vertexVal;
layout(location = 2) in float vertexHeight;

out float fragmentVal;
out vec3 fragmentPos;
out float missingFlag;

uniform mat4 MVP;

#include "schmidt.h"
#include "projection.h"
#include "scale.h"
#include "geometry.h"

out float pointVal;
out vec3 centerVec;
flat out float pointRad;
flat out float fragmentValFlat;

uniform float length10 = 0.01;
uniform float pointSiz = 1.0f;
uniform bool lpointZoo = false;
uniform bool factor = true;

uniform float height_scale = 0.05;

void main ()
{
  vec2 vertexLonLat_ = getVertexLonLat (gl_InstanceID);

  fragmentValFlat = vertexVal;

  vec3 vertexPos = posFromLonLat (vertexLonLat_);
  vec3 pos = vertexPos;

  vec2 pos2 = vec2 (0.0f, 0.0f);

  if (gl_VertexID == 0)
    pos2 = vec2 (-1.0f, -1.0f);
  else if (gl_VertexID == 1)
    pos2 = vec2 (+1.0f, -1.0f);
  else if (gl_VertexID == 2)
    pos2 = vec2 (+1.0f, +1.0f);
  else if (gl_VertexID == 3)
    pos2 = vec2 (-1.0f, +1.0f);
  
  float siz = 0.5 * pointSiz;

  vec3 northPos  = vec3 (0., 0., 1.);
  vec3 vx = normalize (cross (northPos, pos));
  vec3 vy = normalize (cross (pos, vx));

  if (lpointZoo)
    pointRad = siz * 0.02;
  else
    pointRad = siz * length10;

 
  if (factor)
    {
      float f = scalingFactor (compNormedPos (vertexPos));
      pointRad = pointRad / f;
    }

  vec3 normedPos;

  centerVec = pointRad * (pos2.x * vx + pos2.y * vy);
  pos = vertexPos + centerVec;

  normedPos = compNormedPos (pos);
  pos = compProjedPos (vertexPos, normedPos);
  pos = scalePosition (pos, normedPos, scale0);

  if (proj == XYZ)
    pos = pos * (1.0f + height_scale * vertexHeight);


  fragmentPos = normedPos;
  fragmentVal = vertexVal;
  missingFlag = vertexVal == 0 ? 1. : 0.;

  gl_Position =  MVP * vec4 (pos, 1.);

}
