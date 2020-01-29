
#version 330 core

layout(location = 0) in vec2 vertexLonLat;

uniform bool dd;

void main()
{
  if (dd)
  {
  float lon = vertexLonLat.x, lat = vertexLonLat.y;

  float coslon = cos (lon), sinlon = sin (lon);
  float coslat = cos (lat), sinlat = sin (lat);

  vec3 vertexPos = vec3 (coslon * coslat, 
                         sinlon * coslat, 
                                  sinlat);
  gl_Position = vec4 (vertexPos.x, vertexPos.y,
                      vertexPos.z, 1.0f);
 
  }
}
