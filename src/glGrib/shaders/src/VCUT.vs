
#include "version.h"

layout (std430, binding=1) buffer vcutLonLat
{
  float lonlat[];
};

uniform mat4 MVP;

void main()
{
  float lon = lonlat[2*gl_VertexID+0];
  float lat = lonlat[2*gl_VertexID+1];

  float coslon = cos (lon), sinlon = sin (lon);
  float coslat = cos (lat), sinlat = sin (lat);

  vec3 vertexPos = vec3 (coslon * coslat, sinlon * coslat, sinlat);

  gl_Position = MVP * vec4 (vertexPos, 1);
}
