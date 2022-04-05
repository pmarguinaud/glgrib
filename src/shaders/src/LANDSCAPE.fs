
#include "version.h"

in 
#include "LANDSCAPE_VS.h"

out vec4 color;

const int LONLAT = 0;
const int WEBMERCATOR = 1;

uniform int texproj = 0;
uniform sampler2D tex;
uniform vec3 lightDir = vec3 (0., 1., 0.);
uniform vec3 lightCol = vec3 (1., 1., 1.);
uniform bool light = false;
uniform float frac = 0.1;
uniform vec4 color0 = vec4 (0.0f, 0.0f, 0.0f, 0.0f);
uniform bool colored = false;

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


vec4 getColorLATLON (vec3 fragmentPos)
{
  // All lat/lon in radians
  float llon = atan (fragmentPos.y, fragmentPos.x);  
  float llat = asin (fragmentPos.z);
  
  
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
  
  return texture (tex, vec2 (s, t));
}

vec4 getColorMERCATOR (vec3 fragmentPos)
{
  // All lat/lon in radians
  float llon = atan (fragmentPos.y, fragmentPos.x);  
  float llat = asin (fragmentPos.z);

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
  
  return texture (tex, vec2 (X, Y));
}

void main ()
{
  vec4 col = vec4 (0.0, 0.0, 0.0, 0.0);
  
  if (colored)
    {
      col = color0;
    }
  else
    {
      if (texproj == LONLAT)
        {
          col = getColorLATLON (landscape_vs.fragmentPos);
        }
      else if (texproj == WEBMERCATOR)
        {
          col = getColorMERCATOR (landscape_vs.fragmentPos);
        }
    }

  float total = 1.;

  if (light)
    {
      total = frac + (1.0 - frac) * max (dot (landscape_vs.fragmentPos, lightDir), 0.0);
    }

  color.r = total * col.r;
  color.g = total * col.g;
  color.b = total * col.b;
  color.a = 1.;
}
