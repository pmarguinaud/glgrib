

#include "version.h"

layout (location = 0) in vec4  letterPos;
layout (location = 1) in float letterVal;
layout (location = 2) in vec4  letterXYZA;

out
#include "FONT_VS.h"

uniform mat4 MVP;
uniform bool l3d = false;
uniform float length10;
uniform float scaleXYZ = 1.0f;
uniform float ratio = 1.0f;

#include "schmidt.h"
#include "projection.h"
#include "scale.h"

void main ()
{
  vec3 letterXYZ = letterXYZA.xyz;
  float A = letterXYZA.w;

  float xx = letterPos.x;
  float yy = letterPos.y;
  float dx = letterPos.z;
  float dy = letterPos.w;

  vec2 pos2 = vec2 (0.0f, 0.0f);

  if (gl_VertexID == 0)
    pos2 = vec2 (xx     , yy     );
  else if (gl_VertexID == 1)
    pos2 = vec2 (xx + dx, yy     );
  else if (gl_VertexID == 2)
    pos2 = vec2 (xx + dx, yy + dy);
  else if (gl_VertexID == 3)
    pos2 = vec2 (xx     , yy + dy);

  if (l3d) 
    {
      if (proj == XYZ)
        {
          float cosA = cos (A), sinA = sin (A);
          vec3 pos = scaleXYZ * letterXYZ;
          vec3 northPos  = vec3 (0., 0., 1.);
          vec3 vx = normalize (cross (northPos, pos));
          vec3 vy = normalize (cross (pos, vx));

          vec3 wx = + cosA * vx + sinA * vy;
          vec3 wy = - sinA * vx + cosA * vy;
         
          pos = pos + 50 * length10 * (pos2.x * wx + pos2.y * wy);
      
          gl_Position =  MVP * vec4 (pos, 1.);
        }
      else 
        {
          float scale0 = length (letterXYZ);
          vec3 vertexPos = letterXYZ;
          vec3 normedPos = compNormedPos (vertexPos);
          vec3 pos = compProjedPos (vertexPos, normedPos);
          pos = scalePosition (pos, normedPos, scale0);

          gl_Position =  MVP * vec4 (pos, 1.0f);

          if ((proj == POLAR_NORTH) || (proj == POLAR_SOUTH))
            {
              float p = proj == POLAR_NORTH ? +1.0f : -1.0f;
              vec3 posPole = scalePosition (vec3 (+0.0f, +0.0f, +0.0f), 
                                            vec3 (+0.0f, +0.0f,     p),
                                            scale0);
              vec4 pole_gl_Position = MVP * vec4 (posPole, 1.0f);
              A = A + pi / 2.0f + (p - 1.0f) * pi / 2.0f +
                  atan (gl_Position.y - pole_gl_Position.y, 
                        gl_Position.x - pole_gl_Position.x);
            }

          float cosA = cos (A), sinA = sin (A);

          gl_Position.x = gl_Position.x + 10. * (+ cosA * pos2.x - sinA * pos2.y) / ratio;
          gl_Position.y = gl_Position.y + 10. * (+ sinA * pos2.x + cosA * pos2.y);
          gl_Position.z = 0.0f;
        }
    }
  else 
    {
      float cosA = cos (A), sinA = sin (A);
      float Xr = letterXYZ.x, Yr = letterXYZ.y;
      vec2 rpos2 = vec2
      (
        Xr + (+ cosA * (pos2.x - Xr) - sinA * (pos2.y - Yr)),
        Yr + (+ sinA * (pos2.x - Xr) + cosA * (pos2.y - Yr)) 
      );
      gl_Position =  MVP * vec4 (0., rpos2.x, rpos2.y, 1.);
    }
  font_vs.fragmentPos = pos2;
  font_vs.fletterVal  = letterVal;
  font_vs.fletterPos  = vec2 (xx, yy);
}


