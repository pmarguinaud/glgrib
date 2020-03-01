
#include "version.h"

layout (location = 0) in vec2 vertexLonLat;
layout (location = 1) in float vertexHeight;

out vec3 fragmentPos;
uniform mat4 MVP;

#include "projection.h"
#include "scale.h"


uniform mat4 schmidt_rot;
uniform float schmidt_opc2 = 1.25;
uniform float schmidt_omc2 = 0.75;

vec2 doSchmidt (vec2 lonlat)
{
  float lon, lat;

  float coordx = lonlat.x;
  float coordy = lonlat.y;

  float sincoordy = sin (coordy);
  lat = asin ((schmidt_omc2 + sincoordy * schmidt_opc2) 
            / (schmidt_opc2 + sincoordy * schmidt_omc2));
  lon = coordx;

  float coslat = cos (lat), sinlat = sin (lat);
  float coslon = cos (lon), sinlon = sin (lon);
  
  float X = coslon * coslat;
  float Y = sinlon * coslat;
  float Z =          sinlat;
  
  vec4 XYZ = vec4 (X, Y, Z, 0.0f);
  XYZ = schmidt_rot * XYZ;
  
  lon = atan (XYZ.y, XYZ.x);
  lat = asin (XYZ.z);
  
  return vec2 (lon + pi, lat);
}


void main()
{
  vec3 vertexPos = posFromLonLat (vertexLonLat);
  vec3 normedPos = compNormedPos (vertexPos);
  vec3 pos = compProjedPos (vertexPos, normedPos);
  pos = scalePosition (pos, normedPos, scale0);
  pos = pos * (1.0f + vertexHeight);
  gl_Position =  MVP * vec4 (pos, 1.);
  fragmentPos = normedPos;
}
