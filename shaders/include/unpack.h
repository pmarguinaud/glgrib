

float unpack (float v, float vmin, float vmax)
{
  return vmin + (vmax - vmin) * (255.0 * v - 1.0) / 254.0;
}

