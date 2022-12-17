
#include "version.h"

layout(location = 0) in vec2 vertexLonLat;

uniform mat4 MVP;

void main()
{
  float lon = vertexLonLat.x, lat = vertexLonLat.y;

  float coslon = cos (lon), sinlon = sin (lon);
  float coslat = cos (lat), sinlat = sin (lat);

  vec3 vertexPos = vec3 (coslon * coslat, sinlon * coslat, sinlat);

  gl_Position = MVP * vec4 (vertexPos, 1);
}
