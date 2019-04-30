#include "glgrib_load.h"

#include <stdio.h>
#include <stdlib.h>
#include <eccodes.h>
#include <iostream>

void glgrib_load (const char * file, float ** val, int what)
{
  FILE * in = fopen (file, "r");

  *val = NULL;

  int err = 0;
  codes_handle * h = codes_handle_new_from_file (0, in, PRODUCT_GRIB, &err);
  size_t pl_len;
  size_t v_len = 0;
  codes_get_size (h, "pl", &pl_len);
  codes_get_size (h, "values", &v_len);
  long int * pl = (long int *)malloc (sizeof (long int) * pl_len);
  codes_get_long_array (h, "pl", pl, &pl_len);
  

  double vmin, vmax;
  double * v = NULL;
  codes_get_double (h, "maximum", &vmax);
  codes_get_double (h, "minimum", &vmin);
  if (what == 2)
    {
      v = (double *)malloc (sizeof (double) * v_len);
      codes_get_double_array (h, "values", v, &v_len);
    }

  codes_handle_delete (h);

  *val = (float *)malloc (sizeof (float) * v_len);

  for (int jlat = 1, jglo = 0; jlat <= pl_len; jlat++)
    for (int jlon = 1; jlon <= pl[jlat-1]; jlon++, jglo++)
      switch (what)
        {
          case 0: (*val)[jglo] = ((int)(10 * (float)(jlat-1)/(float)pl_len)) / 10.0f; break;
          case 1: (*val)[jglo] = ((int)(10 * (float)(jlon-1)/(float)pl[jlat-1])) / 10.0f; break;
          case 2: (*val)[jglo] = ((int)(10 * (v[jglo] - vmin) / (vmax - vmin))) / 10.0f; break;
        }

  
  free (pl);

  if (v != NULL)
    free (v);

  fclose (in);
}

