
#include "geometry/gaussian/kinds.h"

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
uniform mat3  geometry_gaussian_rot;
uniform bool  geometry_gaussian_rotated;
uniform float geometry_gaussian_omc2;
uniform float geometry_gaussian_opc2;
uniform float geometry_gaussian_latfit_coeff[10];
uniform int   geometry_gaussian_latfit_degre;
uniform int   geometry_gaussian_numberOfPoints;
uniform bool  geometry_gaussian_fitlat;
uniform int   geometry_gaussian_kind;

void geometry_gaussian_miss ()
{
}

#include "geometry/gaussian/guess_jlat.h"

vec2 getGaussianVertexLonLat (int jglo) 
{
  const float twopi = 2.0f * pi;
  int jlat;

  if (geometry_gaussian_fitlat)
    {
      jlat = geometry_gaussian_guess_jlat (jglo);
    }
  else
    {
      jlat = geometry_gaussian_jlat[jglo];
    }

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
  
      vec3 XYZ = vec3 (X, Y, Z);
      XYZ = geometry_gaussian_rot * XYZ;
  
      lon = atan (XYZ.y, XYZ.x);
      lat = asin (XYZ.z);
    }


  return vec2 (lon, lat);
}

