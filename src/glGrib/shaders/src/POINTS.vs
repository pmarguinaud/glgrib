
#include "version.h"

layout (location = 0) in vec3 aPos;

uniform mat4 MVP;

#include "schmidt.h"
#include "projection.h"
#include "scale.h"

uniform float length10 = 0.01;
uniform float valmin;
uniform float valmax;
uniform float pointSiz;
uniform bool lpointSiz;
uniform bool lpointZoo = false;
uniform float ratio = 1.0f;

out float pointVal;
out vec3 centerVec;
flat out float pointRad;

void main()
{

  pointVal = aPos.z;

  vec2 pos2 = vec2 (0.0f, 0.0f);

  if (gl_VertexID == 0)
    pos2 = vec2 (-1.0f, -1.0f);
  else if (gl_VertexID == 1)
    pos2 = vec2 (+1.0f, -1.0f);
  else if (gl_VertexID == 2)
    pos2 = vec2 (+1.0f, +1.0f);
  else if (gl_VertexID == 3)
    pos2 = vec2 (-1.0f, +1.0f);
  
  float lon = aPos.x;
  float lat = aPos.y;
  float siz = 0.5 * pointSiz;
  if (lpointSiz)
    siz = siz * pointVal / valmax;
  
  vec3 pos = posFromLonLat (vec2 (lon, lat));

  if (proj == XYZ)
    {
      pos = applySchmidt (pos);
      pos = pos * scale0;

      vec3 northPos  = vec3 (0., 0., 1.);
      vec3 vx = normalize (cross (northPos, pos));
      vec3 vy = normalize (cross (pos, vx));

      if (lpointZoo)
        pointRad = siz * 0.02;
      else
        pointRad = siz * length10;

      centerVec = pointRad * (pos2.x * vx + pos2.y * vy); 
      pos = pos + centerVec;

      gl_Position = MVP * vec4 (pos, 1.);

    }
  else
    {
      float scale0 = length (pos);
      vec3 vertexPos = pos;
      vec3 normedPos = compNormedPos (vertexPos);
      vec3 pos = compProjedPos (vertexPos, normedPos);
      pos = scalePosition (pos, normedPos, scale0);

      gl_Position =  MVP * vec4 (pos, 1.);

      pointRad = siz * 0.1;
      centerVec.xy = pointRad * pos2;
      centerVec.z = 0.0f;

      gl_Position.x = gl_Position.x + centerVec.x / ratio;
      gl_Position.y = gl_Position.y + centerVec.y;
      gl_Position.z = 0.0f;


    }

}
