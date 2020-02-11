

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
uniform mat4  geometry_gaussian_rot;
uniform bool  geometry_gaussian_rotated;
uniform float geometry_gaussian_omc2;
uniform float geometry_gaussian_opc2;
uniform float geometry_gaussian_latfit_coeff[10];
uniform int   geometry_gaussian_latfit_degre;
uniform int   geometry_gaussian_numberOfPoints;


float geometry_gaussian_guess_lat_lin (int jglo)
{
  return asin (1.0 - 2.0 * float (jglo) / float (geometry_gaussian_numberOfPoints));
}

float geometry_gaussian_latfit_eval (int jlat)
{
  float x = float (jlat);
  if (geometry_gaussian_latfit_degre == 4)
    return geometry_gaussian_latfit_coeff[0] + x *
	  (geometry_gaussian_latfit_coeff[1] + x *
	  (geometry_gaussian_latfit_coeff[2] + x *
	  (geometry_gaussian_latfit_coeff[3] + x *
	  (geometry_gaussian_latfit_coeff[4]))));
  float y = 0;
  for (int i = geometry_gaussian_latfit_degre; i >= 0; i--)
     y = y * x + geometry_gaussian_latfit_coeff[i];
  return y;
}

int geometry_gaussian_guess_jlat (int jglo)
{
  bool south = jglo > geometry_gaussian_numberOfPoints / 2;
  int iglo = south ? geometry_gaussian_numberOfPoints - 1 - jglo : jglo;
  float lat = geometry_gaussian_guess_lat_lin (iglo);
  int ilat0 = int ((0.5 - lat / pi) * (geometry_gaussian_Nj + 1) - 1);
  int ilat1 = int (geometry_gaussian_latfit_eval (ilat0));
  int jlat = ilat0 + ilat1;

  jlat = max (min (jlat, geometry_gaussian_Nj), 0);

  while (iglo < geometry_gaussian_jglooff[jlat])
    jlat = jlat - 1;

  while (iglo >= geometry_gaussian_jglooff[jlat+1])
    jlat = jlat + 1;

  jlat = south ? geometry_gaussian_Nj - 1 - jlat : jlat;

  return jlat;
}

vec2 getGaussianVertexLonLat (int jglo) 
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

