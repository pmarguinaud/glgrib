
uniform mat4 schmidt_rotd;
uniform mat4 schmidt_roti;
uniform float schmidt_opc2 = 1.25;
uniform float schmidt_omc2 = 0.75;
uniform bool schmidt_apply = false;

vec3 applySchmidt (vec3 pos)
{
  if (! schmidt_apply)
    return pos;

  vec4 XYZ = vec4 (pos.x, pos.y, pos.z, 0.0f);
  
  XYZ = schmidt_roti * XYZ;
  XYZ.xyz = XYZ.xyz / length (XYZ.xyz);
  
  float lon = atan (XYZ.y, XYZ.x);
  float lat = asin (max (-1.0, min (1.0, XYZ.z)));
  
  float coordx = lon;
  float coordy = lat;

  float sincoordy = sin (coordy);
  lat = asin ((schmidt_omc2 + sincoordy * schmidt_opc2) 
            / (schmidt_opc2 + sincoordy * schmidt_omc2));
  lon = coordx;

  float coslat = cos (lat), sinlat = sin (lat);
  float coslon = cos (lon), sinlon = sin (lon);
  
  XYZ = vec4 (coslon * coslat, sinlon * coslat, sinlat, 0.0f);
  XYZ = schmidt_rotd * XYZ;
  XYZ.xyz = XYZ.xyz / length (XYZ.xyz);
  
  return XYZ.xyz;
}


