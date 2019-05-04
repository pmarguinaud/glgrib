#include "glgrib_load.h"

#include <stdio.h>
#include <stdlib.h>
#include <eccodes.h>
#include <iostream>
#include <stdexcept>
#include <string>
#include <errno.h>


void glgrib_load (const std::string & file, float ** val, float * valmin, float * valmax, float * valmis)
{
  FILE * in = fopen (file.c_str (), "r");

  if (in == NULL)
    {
      throw std::runtime_error (std::string ("Cannot open ") + file + ": " + strerror (errno));
    }

  *val = NULL;

  int err = 0;
  codes_handle * h = codes_handle_new_from_file (0, in, PRODUCT_GRIB, &err);
  if (h == NULL)
    {
      fclose (in);
      throw std::runtime_error (std::string ("`") + file + "' does not contain GRIB data");
    }

  size_t v_len = 0;
  codes_get_size (h, "values", &v_len);

  double vmis, vmin, vmax;
  codes_get_double (h, "missingValue", &vmis);
  codes_get_double (h, "minimum",      &vmin);
  codes_get_double (h, "maximum",      &vmax);
  double * v = (double *)malloc (sizeof (double) * v_len);
  codes_get_double_array (h, "values", v, &v_len);

  codes_handle_delete (h);

  fclose (in);

  *val = (float *)malloc (sizeof (float) * v_len);
  for (int i = 0; i < v_len; i++)
    (*val)[i] = v[i];

  free (v);

  *valmis = vmis;
  *valmin = vmin;
  *valmax = vmax;

  std::cout << " vmis = " << vmis << std::endl;

  int count = 0;
  for (int i = 0; i < v_len; i++)
    if ((*val)[i] == vmis)
      count++;

  std::cout << " count = " << count << std::endl;

}

