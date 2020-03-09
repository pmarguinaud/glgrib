uniform float geometry_latlon_lat0;
uniform float geometry_latlon_lon0;
uniform float geometry_latlon_dlat;
uniform float geometry_latlon_dlon;
uniform int   geometry_latlon_Ni;
uniform int   geometry_latlon_Nj;

vec2 getLatLonVertexLonLat (int jglo)
{
  int i = jglo % geometry_latlon_Ni;
  int j = jglo / geometry_latlon_Ni;

  float lat = geometry_latlon_lat0 - geometry_latlon_dlat * float (j);
  float lon = geometry_latlon_lon0 + geometry_latlon_dlon * float (i);

  return vec2 (lon, lat);
}  



