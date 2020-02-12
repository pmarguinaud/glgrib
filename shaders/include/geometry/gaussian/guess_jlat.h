float geometry_gaussian_guess_lat_lin (int jglo)
{
  return asin (1.0 - 2.0 * float (jglo) / float (geometry_gaussian_numberOfPoints));
}

float geometry_gaussian_guess_lat_oct (int jglo)
{
  return 0.5 * pi * (1.0 - sqrt (2.0 * float (jglo) / float (geometry_gaussian_numberOfPoints)));
}

float geometry_gaussian_guess_lat (int jglo)
{
  float lat = 0.0;
  switch (geometry_gaussian_kind)
    {
      case geometry_gaussian_kind_lin:
        lat = geometry_gaussian_guess_lat_lin (jglo);
        break;
      case geometry_gaussian_kind_oct:
        lat = geometry_gaussian_guess_lat_oct (jglo);
        break;
    }
  return lat;
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
  else if (geometry_gaussian_latfit_degre == 3)
    return geometry_gaussian_latfit_coeff[0] + x *
	  (geometry_gaussian_latfit_coeff[1] + x *
	  (geometry_gaussian_latfit_coeff[2] + x *
	  (geometry_gaussian_latfit_coeff[3])));
  float y = 0;
  for (int i = geometry_gaussian_latfit_degre; i >= 0; i--)
     y = y * x + geometry_gaussian_latfit_coeff[i];
  return y;
}

int geometry_gaussian_guess_jlat0 (int jglo)
{
  float lat = geometry_gaussian_guess_lat (jglo);
  return int ((0.5 - lat / pi) * (geometry_gaussian_Nj + 1) - 1);
}

int geometry_gaussian_guess_jlat (int jglo)
{
  bool south = jglo > geometry_gaussian_numberOfPoints / 2;
  int iglo = south ? geometry_gaussian_numberOfPoints - 1 - jglo : jglo;

  int ilat0 = geometry_gaussian_guess_jlat0 (iglo);

  // Correction with polynomial
  int ilat1 = int (geometry_gaussian_latfit_eval (ilat0));
  int jlat = ilat0 + ilat1;

  jlat = max (min (jlat, geometry_gaussian_Nj), 0);

  while (iglo < geometry_gaussian_jglooff[jlat])
    {
      jlat = jlat - 1;
      geometry_gaussian_miss ();
    }

  while (iglo >= geometry_gaussian_jglooff[jlat+1])
    {
      jlat = jlat + 1;
      geometry_gaussian_miss ();
    }

  jlat = south ? geometry_gaussian_Nj - 1 - jlat : jlat;

  return jlat;
}


