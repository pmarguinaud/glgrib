
#version 330 core

layout(location = 0) in vec2 vertexLonLat;
layout(location = 1) in float vertexVal_n;
layout(location = 2) in float vertexVal_d;
layout(location = 3) in float vertexHeight;

out vec3 fragmentPos;
out float alpha;


uniform float valmin_n, valmax_n;
uniform float valmin_d, valmax_d;

uniform mat4 MVP;

#include "projection.h"
#include "scale.h"

vec3 vprod (vec3 u, vec3 v)
{
  return vec3 (u.y * v.z - u.z * v.y, 
               u.z * v.x - u.x * v.z, 
               u.x * v.y - u.y * v.x);
}

const float deg2rad = pi / 180.0;

uniform float vscale = 0.01;
uniform float head = 0.1;
uniform float posmax = 0.97;

uniform float height_scale = 0.05;

void main ()
{
  vec3 vertexPos = posFromLonLat (vertexLonLat);

  vec3 u = normalize (vec3 (-vertexPos.y, +vertexPos.x, 0.));
  vec3 v = vprod (vertexPos, u);

  bool defined = vertexVal_d != 0;
  vec3 pos;
  
  if (! defined)
    pos = vec3 (+0.0     ,  +0.0, +0.0);
  else if (gl_VertexID == 0)
    pos = vec3 (+0.0     ,  +0.0, +0.0);
  else if (gl_VertexID == 1)
    pos = vec3 (+1.0     ,  +0.0, +0.0);
  else if (gl_VertexID == 2)
    pos = vec3 (+1.0-head, +head, +0.0);
  else if (gl_VertexID == 3)
    pos = vec3 (+1.0-head, -head, +0.0);
  else if (gl_VertexID == 4)
    pos = vec3 (+1.0     ,  +0.0, +0.0);

  alpha = 1.;
  if (defined)
    {
      float N = valmin_n + (valmax_n - valmin_n) * (255.0 * vertexVal_n - 1.0) / 254.0;
      float D = valmin_d + (valmax_d - valmin_d) * (255.0 * vertexVal_d - 1.0) / 254.0;
      D = D * deg2rad;
      float X = vscale * N * cos (D) / valmax_n;
      float Y = vscale * N * sin (D) / valmax_n;
     
      pos = vertexPos + (pos.x * X - pos.y * Y) * u + (pos.x * Y + pos.y * X) * v;
     
      vec3 normedPos = compNormedPos (pos);
      vec3 projedPos = compProjedPos (pos, normedPos);
      pos = scalePosition (projedPos, normedPos, scale0);

      if (proj == XYZ)
        pos = pos * (1.0f + height_scale * vertexHeight);

      if ((proj == LATLON) || (proj == MERCATOR))
      if ((pos.y < -posmax) || (+posmax < pos.y))
        {
          pos.x = -0.1;
          alpha = 0.0;
	}
    }

  gl_Position =  MVP * vec4 (pos, 1.);

}
