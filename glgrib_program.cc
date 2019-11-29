#include "glgrib_program.h"
#include "glgrib_opengl.h"
#include "glgrib_shader.h"

#include <string>
#include <math.h>
#include <glm/glm.hpp>


static const std::string projShaderInclude = 
R"CODE(
const int XYZ=0;
const int POLAR_NORTH=1;
const int POLAR_SOUTH=2;
const int MERCATOR=3;
const int LATLON=4;
uniform int proj = 0;
uniform bool isflat = true;
uniform float lon0 = 180.0; // Latitude of right handside
const float pi = 3.1415926;

vec3 posFromLonLat (vec2 vertexLonLat)
{
  float lon = vertexLonLat.x, lat = vertexLonLat.y;
  float coslon = cos (lon), sinlon = sin (lon);
  float coslat = cos (lat), sinlat = sin (lat);
  return vec3 (coslon * coslat, sinlon * coslat, sinlat);
}

vec3 compNormedPos (vec3 vertexPos)
{
  float x = vertexPos.x;
  float y = vertexPos.y;
  float z = vertexPos.z;
  float r = 1. / sqrt (x * x + y * y + z * z); 
  return vec3 (x * r, y * r, z * r);
}

vec3 compProjedPos (vec3 vertexPos, vec3 normedPos)
{
  vec3 pos;
  switch (proj)
    {
      case XYZ:
        if (isflat)
          pos = normedPos;
        else
          pos = vertexPos;
        break;
      case POLAR_NORTH:
        pos =  vec3 (0., normedPos.x / (+normedPos.z + 1.0), 
                     normedPos.y / (+normedPos.z + 1.0));
        break;      
      case POLAR_SOUTH:
        pos =  vec3 (0., normedPos.x / (-normedPos.z + 1.0), 
                     normedPos.y / (-normedPos.z + 1.0));
        break;      
      case MERCATOR:
        {
          float lat = asin (normedPos.z);
          float lon = mod (atan (normedPos.y, normedPos.x), 2 * pi);
          float X = (mod (lon - lon0 * pi / 180.0, 2 * pi) - pi) / pi;
          float Y = log (tan (pi / 4. + lat / 2.)) / pi;
          pos = vec3 (0., X, Y);
        }
        break;
      case LATLON:
        {
          float lat = asin (normedPos.z);
          float lon = mod (atan (normedPos.y, normedPos.x), 2 * pi);
          float X = (mod (lon - lon0 * pi / 180.0, 2 * pi) - pi) / pi;
          float Y = lat / pi;
          pos = vec3 (0., X, Y);
        }
        break;
    }

  return pos;
}
)CODE";


static const std::string scalePositionInclude = 
R"CODE(

uniform vec3 scale0 = vec3 (1.0, 1.0, 1.0);

vec3 scalePosition (vec3 pos, vec3 normedPos, vec3 scale0)
{
  if (proj == XYZ)
    {
      pos.x = scale0.x * normedPos.x;
      pos.y = scale0.y * normedPos.y;
      pos.z = scale0.z * normedPos.z;
    }
  else if (proj == POLAR_SOUTH)
    {
      pos.x = pos.x - (scale0.x - 1.0f);
    }
  else
    {
      pos.x = pos.x + (scale0.x - 1.0f);
    }

  return pos;
}

float scalingFactor (vec3 normedPos)
{
  if (proj == POLAR_SOUTH)
    {
      return 1.0 / (1.0 - normedPos.z);
    }
  else if (proj == POLAR_NORTH)
    {
      return 1.0 / (1.0 + normedPos.z);
    }
  else if (proj == MERCATOR)
    {
      return 1.0 / sqrt (1 - normedPos.z * normedPos.z);
    }
  return 1.0;
}


)CODE";


static const std::string enlightFragmentInclude = 
R"CODE(

uniform vec4 RGBA0[256];
uniform float valmin, valmax;
uniform float palmin, palmax;

uniform vec3 lightDir = vec3 (0., 1., 0.);
uniform vec3 lightCol = vec3 (1., 1., 1.);
uniform bool light = false;
uniform float frac = 0.1;
uniform bool smoothed = true;


vec4 enlightFragment (vec3 fragmentPos, float fragmentVal, float missingFlag)
{
  if (missingFlag > 0.)
    discard;

  vec4 color;

  float total = 1.;

  if (light)
    {
      total = frac + (1.0 - frac) * max (dot (fragmentPos, lightDir), 0.0);
    }

  float val = valmin + (valmax - valmin) * (255.0 * fragmentVal - 1.0f) / 254.0f;
  float pal = max (1.0f, min (1.0f + 254.0f * (val - palmin) / (palmax - palmin), 255.0f));

  if (smoothed)
    {
      int pal0 = int (floor (pal)), pal1 = int (ceil (pal));
      bool same = pal0 == pal1;
      float a1 = same ? 1. : pal - pal0;
      float a0 = same ? 0. : pal1 - pal;
      color = RGBA0[pal0] * a0 + RGBA0[pal1] * a1;
    }
  else
    {
      color = RGBA0[int (pal)];
    }

  color.rgb = total * color.rgb;

  return color;
}
)CODE";


static glgrib_program PRG[glgrib_program::SIZE] = 
{
  glgrib_program (  // 3 colors + alpha channel
R"CODE(
#version 330 core

in vec4 fragmentCol;

out vec4 color;

void main()
{
  color.r = fragmentCol.r;
  color.g = fragmentCol.g;
  color.b = fragmentCol.b;
  color.a = fragmentCol.a;
}
)CODE",
R"CODE(
#version 330 core

layout(location = 0) in vec3 vertexPos;
layout(location = 1) in vec4 vertexCol;

out vec4 fragmentCol;
uniform mat4 MVP;

void main()
{
  gl_Position =  MVP * vec4 (vertexPos, 1);
  fragmentCol.r = vertexCol.r;
  fragmentCol.g = vertexCol.g;
  fragmentCol.b = vertexCol.b;
  fragmentCol.a = vertexCol.a;
}
)CODE"),

  glgrib_program (  // 3 colors 
R"CODE(
#version 330 core

in vec3 fragmentCol;

out vec4 color;

void main()
{
  color.r = fragmentCol.r;
  color.g = fragmentCol.g;
  color.b = fragmentCol.b;
  color.a = 255;
}
)CODE",
R"CODE(
#version 330 core

layout(location = 0) in vec3 vertexPos;
layout(location = 1) in vec3 vertexCol;

out vec3 fragmentCol;
uniform mat4 MVP;

void main()
{
  gl_Position =  MVP * vec4 (vertexPos, 1);
  fragmentCol.r = vertexCol.r;
  fragmentCol.g = vertexCol.g;
  fragmentCol.b = vertexCol.b;
}
)CODE"),

  glgrib_program ( // Fixed color
R"CODE(
#version 330 core

in float alpha;

out vec4 color;

uniform vec3 color0; 

void main()
{
  color.r = color0.r;
  color.g = color0.g;
  color.b = color0.b;
  color.a = alpha;

}
)CODE",
R"CODE(
#version 330 core

layout(location = 0) in vec3 vertexPos;
out float alpha;


uniform mat4 MVP;
uniform bool do_alpha = false;
uniform float posmax = 0.97;
uniform float scale = 1.005;

)CODE" + projShaderInclude + R"CODE(

void main()
{
  vec3 pos;

  alpha = 1.0;

  if (proj == XYZ)
    {
      pos = scale * vertexPos;
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

  gl_Position =  MVP * vec4 (pos, 1.);

}
)CODE"),

  glgrib_program (  // 3 colors, flat
R"CODE(
#version 330 core

in vec3 fragmentCol;

out vec4 color;

void main()
{
  color.r = fragmentCol.r;
  color.g = fragmentCol.g;
  color.b = fragmentCol.b;
  color.a = 255;
}
)CODE",
R"CODE(
#version 330 core

layout(location = 0) in vec3 vertexPos;
layout(location = 1) in vec3 vertexCol;

out vec3 fragmentCol;
uniform mat4 MVP;

void main()
{
  float x = vertexPos.x;
  float y = vertexPos.y;
  float z = vertexPos.z;
  float r = 1. / sqrt (x * x + y * y + z * z); 
  vec3 pos;
  pos.x = x * r;
  pos.y = y * r;
  pos.z = z * r;
  gl_Position =  MVP * vec4 (pos, 1);
  fragmentCol.r = vertexCol.r;
  fragmentCol.g = vertexCol.g;
  fragmentCol.b = vertexCol.b;
}
)CODE"),

  glgrib_program (  // 3 colors, position, scale
R"CODE(
#version 330 core

in vec3 fragmentCol;

out vec4 color;

void main()
{
  color.r = fragmentCol.r;
  color.g = fragmentCol.g;
  color.b = fragmentCol.b;
  color.a = 255;
}
)CODE",
R"CODE(
#version 330 core

layout(location = 0) in vec3 vertexPos;
layout(location = 1) in vec3 vertexCol;

out vec3 fragmentCol;
uniform mat4 MVP;
uniform vec3 position0 = vec3 (0.0, 0.0, 0.0);
uniform vec3 scale0 = vec3 (1.0, 1.0, 1.0);

void main()
{
  float x = vertexPos.x;
  float y = vertexPos.y;
  float z = vertexPos.z;
  vec3 pos;
  pos.x = scale0.x * x + position0.x;
  pos.y = scale0.y * y + position0.y;
  pos.z = scale0.z * z + position0.z;
  gl_Position =  MVP * vec4 (pos, 1);
  fragmentCol.r = vertexCol.r;
  fragmentCol.g = vertexCol.g;
  fragmentCol.b = vertexCol.b;
}
)CODE"),

  glgrib_program (  // GRADIENT_FLAT_SCALE_SCALAR
R"CODE(
#version 330 core

in float fragmentVal;
in vec3 fragmentPos;
in float missingFlag;
out vec4 color;

)CODE" +
enlightFragmentInclude +
R"CODE(
void main ()
{
  color = enlightFragment (fragmentPos, fragmentVal, missingFlag);
}
)CODE",
R"CODE(
#version 330 core

layout(location = 0) in vec2 vertexLonLat;
layout(location = 1) in float vertexVal;

out float fragmentVal;
out vec3 fragmentPos;
out float missingFlag;

uniform mat4 MVP;

)CODE" 
+ projShaderInclude 
+ scalePositionInclude 
+ R"CODE(

void main ()
{
  vec3 vertexPos = posFromLonLat (vertexLonLat);
  vec3 normedPos = compNormedPos (vertexPos);
  vec3 pos = compProjedPos (vertexPos, normedPos);
  pos = scalePosition (pos, normedPos, scale0);

  gl_Position =  MVP * vec4 (pos, 1.);

  fragmentVal = vertexVal;
  fragmentPos = normedPos;
  missingFlag = vertexVal == 0 ? 1. : 0.;

}
)CODE"),

  glgrib_program (  // FLAT_TEX
R"CODE(
#version 330 core

in vec3 fragmentPos;
out vec4 color;

const int LONLAT = 0;
const int WEBMERCATOR = 1;

uniform int texproj = 0;
uniform sampler2D texture;
uniform vec3 lightDir = vec3 (0., 1., 0.);
uniform vec3 lightCol = vec3 (1., 1., 1.);
uniform bool light = false;
uniform float frac = 0.1;

// LONLAT
uniform float lonA = -180.0, lonB = +180.0, latA = -90, latB = +90;

// WEBMERCATOR
uniform float F;
uniform int N;
uniform float X0 = -20037508.3427892476320267;
uniform float Y0 = +20037508.3427892476320267;
uniform int IX0;
uniform int IY0;
uniform int IX1;
uniform int IY1;

const float PI = 3.141592653589793;
const float a = 6378137;


void main ()
{
  // All lat/lon in radians
  float llon = atan (fragmentPos.y, fragmentPos.x);  
  float llat = asin (fragmentPos.z);

  vec4 col;

  if (texproj == LONLAT)
    {
      float llonA = lonA, llonB = lonB; 
      float llatA = latA, llatB = latB;
     
      while (llonB < llonA)
        {
          llonB = llonB + 2 * PI;
        }
     
      float s   = (llon - llonA)          / (llonB - llonA);
      float sp1 = (llon - llonA + 2 * PI) / (llonB - llonA);
      float sm1 = (llon - llonA - 2 * PI) / (llonB - llonA);
      float t   = (llat - llatA)          / (llatB - llatA);

      if ((0 <= sp1) && (sp1 <= 1))
        s = sp1;
     
      if ((0 <= sm1) && (sm1 <= 1))
        s = sm1;
     
      if ((s < 0.0) || (1.0 < s))
        discard;
      if ((t < 0.0) || (1.0 < t))
        discard;
     
      col = texture2D (texture, vec2 (s, t));
    }
  else if (texproj == WEBMERCATOR)
    {

      float X = a * llon;
      float Y = a * log (tan (PI / 4. + llat * 0.5));
     
      float Z = 256 * F / N;
     
      float DX = X - X0;
      float DY = Y0 - Y;
     
      float IDX = 1 + IX1 - IX0;
      float IDY = 1 + IY1 - IY0;
     
      int IX = int (DX / Z); 
      int IY = int (DY / Z); 
     
      X = 0 + (DX - IX0 * Z) / (Z * IDX);
      Y = 1 - (DY - IY0 * Z) / (Z * IDY);
     
      float XP1 = X + float (N) / float (IDX);
      float XM1 = X - float (N) / float (IDX);
     
      if ((0 <= XM1) && (XM1 <= 1))
        X = XM1;
     
      if ((0 <= XP1) && (XP1 <= 1))
        X = XP1;

      bool inl = (0 <= X) && (X <= 1) && (0 <= Y) && (Y <= 1);

      if (! inl)
        discard;

      col = texture2D (texture, vec2 (X, Y));

    }

  float total = 1.;

  if (light)
    {
      total = frac + (1.0 - frac) * max (dot (fragmentPos, lightDir), 0.0);
    }

  color.r = total * col.r;
  color.g = total * col.g;
  color.b = total * col.b;
  color.a = 1.;
}
)CODE",
R"CODE(
#version 330 core

layout (location = 0) in vec3 vertexPos;

out vec3 fragmentPos;
uniform mat4 MVP;

)CODE" + projShaderInclude + R"CODE(

void main()
{
  vec3 normedPos = compNormedPos (vertexPos);
  vec3 pos = compProjedPos (vertexPos, normedPos);
  gl_Position =  MVP * vec4 (pos, 1.);
  fragmentPos = normedPos;
}
)CODE"),

  glgrib_program (  // GRADIENT_FLAT_SCALE_VECTOR
R"CODE(
#version 330 core

in float alpha;
out vec4 color;

uniform vec3 color0;

void main ()
{
  if (alpha == 0.)
    discard;
  color.r = color0.r;
  color.g = color0.g;
  color.b = color0.b;
  color.a = 1.;
}

)CODE",
R"CODE(
#version 330 core

layout(location = 0) in vec3 vertexPos;
layout(location = 1) in float vertexVal_n;
layout(location = 2) in float vertexVal_d;

out vec3 fragmentPos;
out float alpha;


uniform float valmin_n, valmax_n;
uniform float valmin_d, valmax_d;

uniform mat4 MVP;

)CODE" 
+ projShaderInclude 
+ scalePositionInclude 
+ R"CODE(

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

void main ()
{
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

      if ((proj == LATLON) || (proj == MERCATOR))
      if ((pos.y < -posmax) || (+posmax < pos.y))
        {
          pos.x = -0.1;
          alpha = 0.0;
	}
    }

  gl_Position =  MVP * vec4 (pos, 1.);

}
)CODE"),

  glgrib_program (  // CONTOUR
R"CODE(
#version 330 core

in float alpha;
in float dist;
out vec4 color;

uniform vec3 color0;
uniform int N = 0;
uniform bool pattern[256];
uniform float length;
uniform bool dash;

void main ()
{
  if (alpha < 1.)
    discard;
  if(! dash)
    {
      color.r = color0.r;
      color.g = color0.g;
      color.b = color0.b;
      color.a = 1.;
    }
  else
    {
      float r = mod (dist / length, 1.0f);
      int k = int (N * r);

      color.r = color0.r;
      color.g = color0.g;
      color.b = color0.b;

      if (pattern[k])
        color.a = 1.;
      else
        color.a = 0.;

if(false){
      if (r > 0.5f)
        {
          color.r = 1.;
          color.g = 0.;
          color.b = 0.;
        }
      else
        {
          color.r = 0.;
          color.g = 1.;
          color.b = 0.;
        }
      color.a = 1.;
}
      
  }
}

)CODE",
R"CODE(
#version 330 core

layout(location = 0) in vec3 vertexPos0;
layout(location = 1) in vec3 vertexPos1;
layout(location = 2) in vec3 vertexPos2;
layout(location = 3) in float norm0;
layout(location = 4) in float norm1;
layout(location = 5) in float dist0;
layout(location = 6) in float dist1;


out float alpha;
out float dist;


uniform mat4 MVP;

)CODE"
+ projShaderInclude
+ scalePositionInclude
+ R"CODE(

uniform bool do_alpha = false;
uniform float posmax = 0.97;
uniform float width = 0.005;

void main ()
{
  vec3 vertexPos;
  vec3 t0 = normalize (vertexPos1 - vertexPos0);
  vec3 t1 = normalize (vertexPos2 - vertexPos1);


  if ((gl_VertexID == 0) || (gl_VertexID == 2))
    vertexPos = vertexPos0;
  else if ((gl_VertexID == 1) || (gl_VertexID == 3) || (gl_VertexID == 5) || (gl_VertexID == 4))
    vertexPos = vertexPos1;  

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

  alpha = min (norm0, norm1);

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

  if ((gl_VertexID == 0) || (gl_VertexID == 2))
    {
      dist = dist0;
    }
  else
    {
      dist = dist1;
    }



}
)CODE"),

  glgrib_program (  // FONT
R"CODE(
#version 330 core

in vec2 fragmentPos;
in vec2 fletterPos;
in float fletterVal;

out vec4 color;

uniform sampler2D texture;

uniform float xoff[30];
uniform float yoff[30];
uniform int nx = 0;
uniform int ny = 0;
uniform float aspect = 1.0;
uniform float scale = 1.0;
uniform vec4 color0;
uniform vec4 color1;

void main ()
{
  float dx = scale * aspect;
  float dy = scale;

  int ix = int (mod (fletterVal, nx));
  int iy = int (fletterVal / nx);

  float tx = (fragmentPos.x - fletterPos.x) / dx;
  float ty = (fragmentPos.y - fletterPos.y) / dy;

  tx = xoff[ix] + tx * (xoff[ix+1] - xoff[ix]);
  ty = yoff[iy] + ty * (yoff[iy+1] - yoff[iy]);

  vec4 col = texture2D (texture, vec2 (tx, ty));


  float a = col.r;

  bool usebg = color1.a > 0.0f;

  if (usebg)
    {
      color.r = color1.r * a + (1.0 - a) * color0.r;
      color.g = color1.g * a + (1.0 - a) * color0.g;
      color.b = color1.b * a + (1.0 - a) * color0.b;
      color.a = color1.a * a + (1.0 - a) * color0.a;
    }
  else
    {
      color.r = color0.r;
      color.g = color0.g;
      color.b = color0.b;
      color.a = 1 - a;
    }
}
)CODE",
R"CODE(

#version 330 core

layout (location = 0) in vec4  letterPos;
layout (location = 1) in float letterVal;
layout (location = 2) in vec4  letterXYZA;

out vec2 fragmentPos;
out vec2 fletterPos;
out float fletterVal;

uniform mat4 MVP;
uniform bool l3d = false;
uniform float length10;

)CODE" 
+ projShaderInclude 
+ scalePositionInclude 
+ R"CODE(

void main ()
{
  vec3 letterXYZ = letterXYZA.xyz;
  float A = letterXYZA.w;
  float cosA = cos (A), sinA = sin (A);

  float xx = letterPos.x;
  float yy = letterPos.y;
  float dx = letterPos.z;
  float dy = letterPos.w;

  vec2 pos2;

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
          vec3 pos = letterXYZ;
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
          float scale00 = length (letterXYZ);
          vec3 scale0 = vec3 (scale00, scale00, scale00);
          vec3 vertexPos = letterXYZ;
          vec3 normedPos = compNormedPos (vertexPos);
          vec3 pos = compProjedPos (vertexPos, normedPos);
          pos = scalePosition (pos, normedPos, scale0);

          gl_Position =  MVP * vec4 (pos, 1.);
       
          gl_Position.x = gl_Position.x + 10. * (+ cosA * pos2.x + sinA * pos2.y);
          gl_Position.y = gl_Position.y + 10. * (- sinA * pos2.x + cosA * pos2.y);
          gl_Position.z = 0.0f;
        }
    }
  else 
    {
      gl_Position =  MVP * vec4 (0., pos2.x, pos2.y, 1.);
    }
  fragmentPos = pos2;
  fletterVal  = letterVal;
  fletterPos  = vec2 (xx, yy);
}


)CODE"),


  glgrib_program (  // IMAGE
R"CODE(
#version 330 core

in vec2 fragmentTex;
out vec4 color;

uniform sampler2D texture;

void main ()
{
  color = texture2D (texture, fragmentTex);
}
)CODE",
R"CODE(

#version 330 core

out vec2 fragmentTex;

uniform mat4 MVP;
uniform float x0;
uniform float y0;
uniform float x1;
uniform float y1;

void main ()
{
  vec2 pos2;

  if (gl_VertexID == 0)
    pos2 = vec2 (0.0f, 0.0f);
  else if (gl_VertexID == 1)
    pos2 = vec2 (1.0f, 0.0f);
  else if (gl_VertexID == 2)
    pos2 = vec2 (1.0f, 1.0f);
  else if (gl_VertexID == 3)
    pos2 = vec2 (0.0f, 1.0f);

  fragmentTex = pos2;

  pos2 = vec2 (x0, y0) + vec2 (x1 - x0, y1 - y0) * pos2;

  gl_Position =  MVP * vec4 (0., pos2.x, pos2.y, 1.);

}


)CODE"),

  glgrib_program (  // POINTS
R"CODE(
#version 330 core
in float pointVal;
in vec3 centerVec;
flat in float pointRad;
out vec4 color;

uniform vec4 RGBA0[256];
uniform vec4 color0;
uniform float palmin;
uniform float palmax;

uniform bool lcolor0;


void main()
{
  if (lcolor0)
    {
      color.r = color0.r;
      color.g = color0.g;
      color.b = color0.b;
      color.a = color0.a;
    }
  else
    {
      int pal = max (1, min (int (1 + 254 * (pointVal - palmin) / (palmax - palmin)), 255));
      color.r = RGBA0[pal].r;
      color.g = RGBA0[pal].g;
      color.b = RGBA0[pal].b;
      color.a = RGBA0[pal].a;
    }
  if (color.r == 0. && color.g == 0. 
   && color.b == 0. && color.a == 0.)
    discard;

  if(false){
  // Round shape
  if (length (centerVec) > pointRad)
    discard;
  }else if(false){
  // Diamond shape
  if (abs (centerVec.x) + abs (centerVec.y) + abs (centerVec.z) > pointRad)
    discard;
  }

}
)CODE",
R"CODE(
#version 330 core

layout (location = 0) in vec3 aPos;

uniform mat4 MVP;

)CODE"
+ projShaderInclude 
+ scalePositionInclude 
+ R"CODE(

uniform float length10 = 0.01;
uniform float valmin;
uniform float valmax;
uniform float pointSiz;
uniform bool lpointSiz;
uniform bool lpointZoo = false;

out float pointVal;
out vec3 centerVec;
flat out float pointRad;

void main()
{

  pointVal = aPos.z;

  vec2 pos2; 

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
  float coslon = cos (lon), sinlon = sin (lon);
  float coslat = cos (lat), sinlat = sin (lat);
  
  vec3 pos = vec3 (coslon * coslat, sinlon * coslat, sinlat);
  pos = pos * scale0;

  if (proj == XYZ)
    {
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
      float scale00 = length (pos);
      vec3 scale0 = vec3 (scale00, scale00, scale00);
      vec3 vertexPos = pos;
      vec3 normedPos = compNormedPos (vertexPos);
      vec3 pos = compProjedPos (vertexPos, normedPos);
      pos = scalePosition (pos, normedPos, scale0);

      gl_Position =  MVP * vec4 (pos, 1.);

      pointRad = siz * 0.1;
      centerVec.xy = pointRad * pos2;
      centerVec.z = 0.0f;

      gl_Position.x = gl_Position.x + centerVec.x;
      gl_Position.y = gl_Position.y + centerVec.y;
      gl_Position.z = 0.0f;


    }

}
)CODE"),

  glgrib_program ( // GRID
R"CODE(
#version 330 core

in float dashed;
in vec3 vertexPos;

out vec4 color;

uniform vec3 color0; 
uniform int frag_resolution = 10;
uniform int frag_nn = 100;
uniform int frag_do_lat = 0;
uniform float dash_length = 4;

const float pi = 3.1415926;
const float rad2deg = 180.0 / pi;

void main()
{
  int k = gl_PrimitiveID;

  float lat = asin (vertexPos.z);
  float lon = mod (atan (vertexPos.y, vertexPos.x), 2 * pi);

  if (frag_do_lat == 0)
    {
      if ((k + 1) % (frag_nn + 1) == 0)
        discard;
      if (dashed > 0.0f)
        {
          if (mod (cos (lat) * lon * rad2deg, dash_length) < dash_length / 2.0)
            discard;
        }
    }
  else
    {
      if ((k + 1) % (frag_nn / 2 + 1) == 0)
        discard;
      if (dashed > 0.0f)
        {
          if (mod (lat * rad2deg, dash_length) < dash_length / 2.0)
            discard;
        }
    }


  color.r = color0.r;
  color.g = color0.g;
  color.b = color0.b;
  color.a = 1.0f;

}
)CODE",
R"CODE(
#version 330 core

out float dashed;
out vec3 vertexPos;

uniform mat4 MVP;
uniform bool do_alpha = false;
uniform float posmax = 0.97;
uniform float scale = 1.005;
uniform int resolution = 10;
uniform int nn = 100;
uniform int do_lat = 0;
uniform int interval = 4;


)CODE" + projShaderInclude + R"CODE(

void main()
{
  vec3 pos;

  int ilat, ilon, nlat, nlon;
  if (do_lat == 0)
    {
      nlat = resolution;
      nlon = nn;
      ilat = gl_VertexID / (nn + 1) + 1;
      ilon = gl_VertexID % (nn + 1);

      int jlat = nlat / 2;
      if ((2 * (nlat / 2) != nlat) && (ilat > nlat / 2))
        jlat = jlat + 1;

      dashed = (abs (ilat - jlat) % interval) != 0 ? 1.0f : 0.0f;
    }
  else
    {
      nlon = resolution * 2;
      nlat = nn / 2;
      ilon = gl_VertexID / (nn / 2 + 1);
      ilat = gl_VertexID % (nn / 2 + 1);
      dashed = (ilon % interval) != 0 ? 1.0f : 0.0f;
    }


  float lon = 2.0f * pi * float (ilon) / float (nlon);
  float lat = 0.5f * pi * float (ilat) / float (nlat / 2.0f) - 0.5 * pi;
  float coslon = cos (lon), sinlon = sin (lon);
  float coslat = cos (lat), sinlat = sin (lat);

  vertexPos = vec3 (coslon * coslat, sinlon * coslat, sinlat);

  if (proj == XYZ)
    {
      pos = scale * vertexPos;
    }
  else
    {
      vec3 normedPos = compNormedPos (vertexPos);
      pos = compProjedPos (vertexPos, normedPos);

      if ((proj == LATLON) || (proj == MERCATOR))
      if ((pos.y < -posmax) || (+posmax < pos.y))
        {
          pos.x = -0.1;
	}

      if (proj == POLAR_SOUTH)
        pos.x = pos.x - 0.005;
      else
        pos.x = pos.x + 0.005;
    }

  gl_Position =  MVP * vec4 (pos, 1.);


}
)CODE"),

  glgrib_program (  // STREAM
R"CODE(
#version 330 core

in float alpha;
in float dist;
in float norm;
out vec4 color;

uniform vec4 RGBA0[256];
uniform float palmin;
uniform float palmax;
uniform float valmin;
uniform float valmax;

void main ()
{
  if (alpha == 0.0f)
    discard;

  vec3 grey = vec3 (0.3f, 0.3f, 0.3f);
  vec3 green= vec3 (0.0f, 1.0f, 0.0f);

  float n = norm / valmax;
  int k = min (255, 1 + int (n * 254.0));
  color = RGBA0[k];

}

)CODE",
R"CODE(
#version 330 core

layout(location = 0) in vec3 vertexPos0;
layout(location = 1) in vec3 vertexPos1;
layout(location = 2) in vec3 vertexPos2;
layout(location = 3) in float norm0;
layout(location = 4) in float norm1;
layout(location = 5) in float dist0;
layout(location = 6) in float dist1;


out float alpha;
out float dist;
out float norm;


uniform mat4 MVP;
uniform float normmax;

)CODE"
+ projShaderInclude
+ scalePositionInclude
+ R"CODE(

uniform bool do_alpha = false;
uniform float posmax = 0.97;
uniform float width;

void main ()
{
  vec3 vertexPos;
  vec3 t0 = normalize (vertexPos1 - vertexPos0);
  vec3 t1 = normalize (vertexPos2 - vertexPos1);


  if ((gl_VertexID == 0) || (gl_VertexID == 2))
    vertexPos = vertexPos0;
  else if ((gl_VertexID == 1) || (gl_VertexID == 3) || (gl_VertexID == 5) || (gl_VertexID == 4))
    vertexPos = vertexPos1;  

  vec3 p = normalize (vertexPos);
  vec3 n0 = cross (t0, p);
  vec3 n1 = cross (t1, p);

  float c = width * max (0.5f, 2.0f * norm0 / normmax) / scalingFactor (p);

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

  norm = alpha = min (norm0, norm1);

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

  if ((gl_VertexID == 0) || (gl_VertexID == 2))
    {
      dist = dist0;
    }
  else
    {
      dist = dist1;
    }



}
)CODE"),

  glgrib_program (  // SCALAR_POINTS
R"CODE(
#version 330 core

in float fragmentVal;
in vec3 fragmentPos;
in float missingFlag;
in float pointVal;
in vec3 centerVec;
flat in float pointRad;
out vec4 color;

)CODE" +
enlightFragmentInclude +
R"CODE(
void main ()
{
//if (length (centerVec) > pointRad)
//  discard;
//if (abs (centerVec.x) + abs (centerVec.y) + abs (centerVec.z) > pointRad)
//  discard;
  color = enlightFragment (fragmentPos, fragmentVal, missingFlag);
}
)CODE",
R"CODE(
#version 330 core

layout(location = 0) in vec3 vertexPos;
layout(location = 1) in float vertexVal;

out float fragmentVal;
out vec3 fragmentPos;
out float missingFlag;

uniform mat4 MVP;

)CODE" 
+ projShaderInclude 
+ scalePositionInclude 
+ R"CODE(

out float pointVal;
out vec3 centerVec;
flat out float pointRad;

uniform float length10 = 0.01;
uniform float pointSiz = 1.0f;
uniform bool lpointZoo = false;
uniform bool factor = true;

void main ()
{
  vec3 pos = vertexPos;

  vec2 pos2; 

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

  fragmentPos = normedPos;
  fragmentVal = vertexVal;
  missingFlag = vertexVal == 0 ? 1. : 0.;

  gl_Position =  MVP * vec4 (pos, 1.);

}
)CODE"),

  glgrib_program (  // TEST
R"CODE(
#version 330 core

out vec4 color;

void main()
{
  color.r = 0.;
  color.g = 1.;
  color.b = 0.;
  color.a = 1.;
}
)CODE",
R"CODE(
#version 330 core

layout(location = 0) in vec3 vertexPos;

uniform mat4 MVP;

void main()
{
  gl_Position = MVP * vec4 (vertexPos, 1);
}
)CODE"),


};

void glgrib_program::compile ()
{
  if (loaded) 
    return;
  programID = glgrib_load_shader (FragmentShaderCode, VertexShaderCode, GeometryShaderCode);
  matrixID = glGetUniformLocation (programID, "MVP");
  loaded = true;
}

glgrib_program * glgrib_program_load (glgrib_program::kind kind)
{
  if (! PRG[kind].loaded)
    PRG[kind].compile ();
  return &PRG[kind];
}

glgrib_program::~glgrib_program ()
{
  if (loaded)
    glDeleteProgram (programID);
}


void glgrib_program::use () const
{
  glUseProgram (programID);
  if (! active)
    {
      glUseProgram (programID);
      active = true;
      for (int i = 0; i < glgrib_program::SIZE; i++) 
        if (PRG[i].programID != programID)
          PRG[i].active = false;
    }
}

void glgrib_program::setLight (const glgrib_options_light & light)
{
  int lightid = glGetUniformLocation (programID, "light");

  if (lightid != -1)
    {
      glUniform1i (lightid, light.on);
      if (light.on)
        {
          const double deg2rad = M_PI / 180.0;
          float coslon = cos (deg2rad * light.lon);
          float sinlon = sin (deg2rad * light.lon);
          float coslat = cos (deg2rad * light.lat);
          float sinlat = sin (deg2rad * light.lat);
          glm::vec3 lightDir = glm::vec3 (coslon * coslat, sinlon * coslat, sinlat);
          glUniform3fv (glGetUniformLocation (programID, "lightDir"), 1, &lightDir[0]);
          glUniform1f (glGetUniformLocation (programID, "frac"), light.night);
        }
    }
}

void glgrib_program::set1f (const std::string & key, float val)
{
  int id = glGetUniformLocation (programID, key.c_str ());
  if (id != -1)
    glUniform1f (id, val);
}

void glgrib_program::set1fv (const std::string & key, const float * val, int size)
{
  int id = glGetUniformLocation (programID, key.c_str ());
  if (id != -1)
    glUniform1fv (id, size, val);
}

void glgrib_program::set1iv (const std::string & key, const int * val, int size)
{
  int id = glGetUniformLocation (programID, key.c_str ());
  if (id != -1)
    glUniform1iv (id, size, val);
}

void glgrib_program::set1i (const std::string & key, int val)
{
  int id = glGetUniformLocation (programID, key.c_str ());
  if (id != -1)
    glUniform1i (id, val);
}

void glgrib_program::set3fv (const std::string & key, const float * val, int size)
{
  int id = glGetUniformLocation (programID, key.c_str ());
  if (id != -1)
    glUniform3fv (id, size, val);
}

void glgrib_program::set4fv (const std::string & key, const float * val, int size)
{
  int id = glGetUniformLocation (programID, key.c_str ());
  if (id != -1)
    glUniform4fv (id, size, val);
}

void glgrib_program::setMatrix4fv (const std::string & key, const float * val, int size)
{
  int id = glGetUniformLocation (programID, key.c_str ());
  if (id != -1)
    glUniformMatrix4fv (id, size, GL_FALSE, val);
}



