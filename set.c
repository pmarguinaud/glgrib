#include <stdio.h>
#include <stdlib.h>
#include "grib_api.h"
 
int main (int argc, char * argv[])
{
  if (argc == 1)
    {
      fprintf (stderr, "Usage: %s in v out\n", argv[0]);
      return 0;
    }

  int i, err;
  double * values;
  size_t len;
  grib_handle * h;
  FILE * fp = fopen (argv[1], "r");
  double v = atof (argv[2]);

  h = grib_handle_new_from_file (0, fp, &err);
  fclose (fp);

  grib_get_size (h, "values", &len);
   
  values = malloc (len * sizeof (double));

  for (int i = 0; i < len; i++)
    values[i] = v;


  grib_set_double_array (h, "values", values, len);

  grib_write_message (h, argv[3], "w");

  free(values);

  grib_handle_delete(h);

  return 0;
}


