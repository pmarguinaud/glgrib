
uniform mat3 schmidt_rotd;
uniform mat3 schmidt_roti;
uniform float schmidt_opc2 = 1.25;
uniform float schmidt_omc2 = 0.75;
uniform bool schmidt_apply = false;

vec3 applySchmidt (vec3 pos)
{
  if (! schmidt_apply)
    return pos;

  vec3 XYZ = schmidt_roti * pos;
  XYZ = XYZ / length (XYZ);
  
  float lon = atan (XYZ.y, XYZ.x);
  float lat = asin (XYZ.z);
  
  float coordx = lon;
  float coordy = lat;

  float sincoordy = sin (coordy);

  float w = (schmidt_omc2 + sincoordy * schmidt_opc2) 
          / (schmidt_opc2 + sincoordy * schmidt_omc2);
  lat = asin (min (+1.0, max (-1.0, w)));
  lon = coordx;

  float coslat = cos (lat), sinlat = sin (lat);
  float coslon = cos (lon), sinlon = sin (lon);
  
  XYZ = vec3 (coslon * coslat, sinlon * coslat, sinlat);
  XYZ = schmidt_rotd * XYZ;
  XYZ = XYZ / length (XYZ);
  
  return XYZ;
}


