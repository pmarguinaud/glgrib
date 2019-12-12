
//uniform int Nmax = 255;
  uniform int Nmax = 65535;

float unpack (float v, float vmin, float vmax)
{
  return vmin + (vmax - vmin) * (Nmax * v - 1) / (Nmax - 1);
}

