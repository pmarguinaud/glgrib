
#include "geometry/buffer_index.h"

layout (std430, binding=geometry_gaussian_jlat_idx) buffer geometry_gaussian1
{
  int geometry_gaussian_jlat[];
};

layout (std430, binding=geometry_gaussian_jglo_idx) buffer geometry_gaussian2
{
  int geometry_gaussian_jglooff[];
};

layout (std430, binding=geometry_gaussian_glat_idx) buffer geometry_gaussian3
{
  float geometry_gaussian_latgauss[];
};

uniform int   geometry_gaussian_Nj;
uniform float geometry_gaussian_stretchingFactor = 1.0f;
uniform mat4  geometry_gaussian_rot;
uniform bool  geometry_gaussian_rotated;
uniform float geometry_gaussian_omc2;
uniform float geometry_gaussian_opc2;

vec2 getVertexLonlat (int jglo) 
{
  const float twopi = 2.0f * pi;
  int jlat = geometry_gaussian_jlat[jglo];
  int jlon = jglo - geometry_gaussian_jglooff[jlat];

  float coordy = geometry_gaussian_latgauss[jlat];
  int pl = geometry_gaussian_jglooff[jlat+1] - geometry_gaussian_jglooff[jlat];
  float coordx = (twopi * float (jlon)) / float (pl);

  float lon, lat;

  if (! geometry_gaussian_rotated)
    {
      lon = coordx;
      lat = coordy;
    }
  else
    {
      float sincoordy = sin (coordy);
      lat = asin ((geometry_gaussian_omc2 + sincoordy * geometry_gaussian_opc2) 
                / (geometry_gaussian_opc2 + sincoordy * geometry_gaussian_omc2));
      lon = coordx;

      float coslat = cos (lat), sinlat = sin (lat);
      float coslon = cos (lon), sinlon = sin (lon);
  
      float X = coslon * coslat;
      float Y = sinlon * coslat;
      float Z =          sinlat;
  
      vec4 XYZ = vec4 (X, Y, Z, 0.0f);
      XYZ = geometry_gaussian_rot * XYZ;
  
      lon = atan (XYZ.y, XYZ.x);
      lat = asin (XYZ.z);
    }


  return vec2 (lon, lat);
}

