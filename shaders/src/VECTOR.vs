#include "version.h"

layout(location = 0) in vec2 vertexLonLat;
layout(location = 1) in float vertexVal_n;
layout(location = 2) in float vertexVal_d;
layout(location = 3) in float vertexHeight;

out float alpha;
out float fragmentVal;
out vec3 fragmentPos;


uniform float valmin_n, valmax_n;
uniform float valmin_d, valmax_d;

uniform mat4 MVP;

#include "projection.h"
#include "scale.h"
#include "unpack.h"
#include "geometry.h"

const float deg2rad = pi / 180.0;

uniform float vscale = 0.01;
uniform bool  arrow_fixed = false;
uniform float head = 0.1;
uniform float posmax = 0.97;

uniform float height_scale = 0.05;
uniform int arrow_kind = 1;
uniform float arrow_min = 0.;

vec3 getPos0 ()
{
  vec3 pos;
       if (gl_VertexID == 0) pos = vec3 (+0.0     ,  +0.0, +0.0);
  else if (gl_VertexID == 1) pos = vec3 (+1.0     ,  +0.0, +0.0);
  else if (gl_VertexID == 2) pos = vec3 (+1.0-head, +head, +0.0);
  else if (gl_VertexID == 3) pos = vec3 (+1.0-head, -head, +0.0);
  else if (gl_VertexID == 4) pos = vec3 (+1.0     ,  +0.0, +0.0);
  return pos;
}

vec3 getPos1 ()
{
  vec3 pos;
       if (gl_VertexID == 0) pos = vec3 (+0.0     ,  +0.0, +0.0);
  else if (gl_VertexID == 1) pos = vec3 (+0.0     , +head, +0.0);
  else if (gl_VertexID == 2) pos = vec3 (+1.0     ,  +0.0, +0.0);
  else if (gl_VertexID == 3) pos = vec3 (+1.0-head, +head, +0.0);
  else if (gl_VertexID == 4) pos = vec3 (+1.0-head, -head, +0.0);
  else if (gl_VertexID == 5) pos = vec3 (+1.0     ,  +0.0, +0.0);
  else if (gl_VertexID == 6) pos = vec3 (+0.0     , -head, +0.0);
  else if (gl_VertexID == 7) pos = vec3 (+0.0     ,  +0.0, +0.0);
  return pos;
}

vec3 getPos2 ()
{
  vec3 pos;
       if (gl_VertexID == 0) pos = vec3 (+0.0     ,  +0.0, +0.0);
  else if (gl_VertexID == 1) pos = vec3 (+0.0     , +head, +0.0);
  else if (gl_VertexID == 2) pos = vec3 (+1.0     ,  +0.0, +0.0);
  else if (gl_VertexID == 3) pos = vec3 (+0.0     , -head, +0.0);
  else if (gl_VertexID == 4) pos = vec3 (+0.0     ,  +0.0, +0.0);
  return pos;
}


uniform float barbxleng;
uniform float barbyleng;
uniform float barbdleng;

uniform float circthres;
uniform int barbthresmax;
uniform float barbthres[10];
uniform int pennthresmax;
uniform float pennthres[10];

vec3 getPos3 (float N)
{

  const int barbvertexmin = 2;
  const int barbvertexmax = barbvertexmin + barbthresmax * 2 - 1;
  const int linevertexmin = barbvertexmax + 1;
  const int linevertexmax = linevertexmin + 1;
  const int pennvertexmin = linevertexmax + 1;
  const int pennvertexmax = pennvertexmin + pennthresmax * 4 - 1;
  const int vertexmax = pennvertexmax;

  vec3 pos = vec3 (0.0f, 0.0f, 0.0f);

//if (gl_InstanceID != 124254)
//  return pos;


  // Circle
  if (N < circthres)
    {
      int k = gl_VertexID;
      k = k + (k % 2);
      float ang = float (k) * pi / (0.5f * (vertexmax - 1));
      pos = barbdleng * vec3 (cos (ang), sin (ang), 0.0f);
      return pos;
    }


  // Number of pennants
  int npenn = 0;

  for (int i = 0; i < pennthresmax; i++)
    if (N >= pennthres[i])
      npenn = i;  
    else
      break;

  N = N - pennthres[npenn];

  // Barb line
  if ((0 <= gl_VertexID) && (gl_VertexID <= 1))
    {
      pos = vec3 (-gl_VertexID, +0.0, +0.0);
    }
  // Barb
  else if ((barbvertexmin <= gl_VertexID) && (gl_VertexID <= barbvertexmax))
    {
      // Barb rank
      int barbind = (gl_VertexID-barbvertexmin) / 2;
      // Point id (=0, or =1)
      int barbext = (gl_VertexID-barbvertexmin) % 2;

      float Nmax = barbthres[barbind+1];
      float Nmin = barbthres[barbind+0];

      // Barb length
      float n = min (max (0, N - Nmin), Nmax - Nmin) / (Nmax - Nmin);

      pos = vec3 (-1.0 + barbind * barbdleng - n * barbxleng * barbext, 
                  n * barbyleng * barbext, +0.0);
    }
  // Extend barb line
  else if ((linevertexmin <= gl_VertexID) && (gl_VertexID <= linevertexmax) && (npenn > 0))
    {
      if (gl_VertexID == 12)
        pos = vec3 (-1.0f, 0.0f, 0.0f);
      else if (gl_VertexID == 13)
        pos = vec3 (-1.0f - barbxleng, 0.0f, 0.0f);
    }
  // Pennants
  else if ((pennvertexmin <= gl_VertexID) && (gl_VertexID <= pennvertexmax))
    {
      // Pennant rank
      int pennind = (gl_VertexID-pennvertexmin) / 4;
      // Point id (0 <= ... <= 3)
      int pennext = (gl_VertexID-pennvertexmin) % 4;

      // If pennant visible
      if (pennind < npenn)
        {
          if (pennext == 0)
            pos = vec3 (-1.0 - (pennind + 0) * barbxleng - barbxleng, 0.0f, 0.0f); 
          else if (pennext == 1)                        
            pos = vec3 (-1.0 - (pennind + 1) * barbxleng - barbxleng, 0.0f, 0.0f); 
          else if (pennext == 2)                        
            pos = vec3 (-1.0 - (pennind + 1) * barbxleng - barbxleng, barbyleng, 0.0f); 
          else if (pennext == 3)                        
            pos = vec3 (-1.0 - (pennind + 1) * barbxleng - barbxleng + barbxleng, 
                        0.0f, 0.0f); 
        }
    }

  return pos;
}

void main ()
{
  vec2 vertexLonLat_ = getVertexLonLat (gl_InstanceID);
  vec3 vertexPos = posFromLonLat (vertexLonLat_);

  vec3 u = normalize (vec3 (-vertexPos.y, +vertexPos.x, 0.));
  vec3 v = cross (vertexPos, u);

  fragmentVal = vertexVal_n;
  fragmentPos = vertexPos;

  float N = unpack (vertexVal_n, valmin_n, valmax_n);
  float D = unpack (vertexVal_d, valmin_d, valmax_d);

  bool defined = (vertexVal_d != 0) && (N > arrow_min);
//defined = gl_InstanceID == 124254;
  vec3 pos;

  if (! defined)
    pos = vec3 (+0.0, +0.0, +0.0);
  else if (arrow_kind == 0)
    pos = getPos0 ();
  else if (arrow_kind == 1)
    pos = getPos1 ();
  else if (arrow_kind == 2)
    pos = getPos2 ();
  else if (arrow_kind == 3)
    pos = getPos3 (N);
  else 
    pos = getPos0 ();
    

  alpha = 1.;
  if (defined)
    {
      D = D * deg2rad;

      float X, Y;

      if (arrow_fixed)
        {
          X = vscale * cos (D);
          Y = vscale * sin (D);
        }
      else
        {
          X = vscale * N * cos (D) / valmax_n;
          Y = vscale * N * sin (D) / valmax_n;
        }
     
      float s = scalingFactor (compNormedPos (vertexPos));

      pos = vertexPos + ((pos.x * X - pos.y * Y) * u + (pos.x * Y + pos.y * X) * v) / s;
     
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
