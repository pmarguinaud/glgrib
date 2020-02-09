
#version 430 core

layout(location = 0) in vec2  vertexLonLat;
layout(location = 1) in float vertexVal;
layout(location = 2) in float vertexHeight;
layout(location = 3) in vec3  vertexMPIView;

out float fragmentVal;
out float fragmentMPI;
out vec3 fragmentPos;
out float missingFlag;
flat out float fragmentValFlat;
flat out float fragmentMPIFlat;

uniform mat4 MVP;
uniform float height_scale = 0.05;
uniform float mpiview_scale = 0.0f;

#include "projection.h"
#include "scale.h"

const float rad2deg = 180.0 / pi;
const float deg2rad = pi / 180.0;

// Start geometry

layout (std430, binding=1) buffer glgrib_geometry_gaussian1
{
  int glgrib_geometry_gaussian_jlat[];
};

layout (std430, binding=2) buffer glgrib_geometry_gaussian2
{
  int glgrib_geometry_gaussian_jglooff[];
};

layout (std430, binding=3) buffer glgrib_geometry_gaussian3
{
  float glgrib_geometry_gaussian_latgauss[];
};

layout (std430, binding=4) buffer glgrib_geometry_gaussian4
{
  int glgrib_geometry_gaussian_pl[];
};

uniform int   glgrib_geometry_gaussian_Nj;
uniform float glgrib_geometry_gaussian_stretchingFactor = 1.0f;
uniform mat4  glgrib_geometry_gaussian_rot;
uniform bool  glgrib_geometry_gaussian_rotated;
uniform float glgrib_geometry_gaussian_omc2;
uniform float glgrib_geometry_gaussian_opc2;

vec2 getVertexLonlat (int jglo) // gl_VertexID
{

  const float twopi = 2.0f * pi;
  int jlat = glgrib_geometry_gaussian_jlat[jglo];
  int jlon = jglo - glgrib_geometry_gaussian_jglooff[jlat];

  float coordy = glgrib_geometry_gaussian_latgauss[jlat];
  float coordx = (twopi * float (jlon)) / float (glgrib_geometry_gaussian_pl[jlat]);

  float lon, lat;


  if (! glgrib_geometry_gaussian_rotated)
    {
      lon = coordx;
      lat = coordy;
    }
  else
    {
      float sincoordy = sin (coordy);
      lat = asin ((glgrib_geometry_gaussian_omc2 + sincoordy * glgrib_geometry_gaussian_opc2) 
                / (glgrib_geometry_gaussian_opc2 + sincoordy * glgrib_geometry_gaussian_omc2));
      lon = coordx;

      float coslat = cos (lat), sinlat = sin (lat);
      float coslon = cos (lon), sinlon = sin (lon);
  
      float X = coslon * coslat;
      float Y = sinlon * coslat;
      float Z =          sinlat;
  
      vec4 XYZ = vec4 (X, Y, Z, 0.0f);
      XYZ = glgrib_geometry_gaussian_rot * XYZ;
  
      lon = atan (XYZ.y, XYZ.x);
      lat = asin (XYZ.z);
    }


  return vec2 (lon, lat);
}

// End geometry


void main ()
{
  vec3 vertexDisp = vec3 (0.0f, 0.0f, 0.0f);

  vec2 vertexLonLat_ = getVertexLonlat (gl_VertexID);

  if (mpiview_scale > 0.0f)
    {
      float lon = vertexMPIView.y, lat = vertexMPIView.z;
      float coslon = cos (lon), sinlon = sin (lon);
      float coslat = cos (lat), sinlat = sin (lat);
      vertexDisp = vec3 (coslon * coslat, sinlon * coslat, sinlat);
    }

  fragmentValFlat = vertexVal;
  fragmentMPIFlat = vertexMPIView.x;

  vec3 vertexPos = posFromLonLat (vertexLonLat_);
  vec3 normedPos = compNormedPos (vertexPos);
  vec3 pos = compProjedPos (vertexPos, normedPos);
  pos = scalePosition (pos, normedPos, scale0);

  if (proj_vs == XYZ)
    {
      pos = pos * (1.0f + height_scale * vertexHeight);
      if (mpiview_scale > 0.0f)
        pos = pos + mpiview_scale * vertexDisp;
    }
    
  gl_Position =  MVP * vec4 (pos, 1.);

  fragmentVal = vertexVal;
  fragmentMPI = vertexMPIView.x;
  fragmentPos = normedPos;
  missingFlag = vertexVal == 0 ? 1. : 0.;

}
