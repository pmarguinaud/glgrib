/*
 * C Implementation: set_data
 *
 * Description: set the data contained in a GRIB file
 *
 */
#include <stdio.h>
#include <stdlib.h>
 
#include "grib_api.h"
#include "eccodes.h"
 
int main(int argc, char** argv)
{
    int i;
    double *values = NULL;
    size_t values_len= 0;
 
    grib_handle *h = NULL;
    double d,e;
    long count;
 
    int err;
    FILE * in = fopen (argv[1], "r");
    h = grib_handle_new_from_file (0,in,&err);
 
    GRIB_CHECK(grib_get_size(h, "values", &values_len),0);
     
    values = malloc(values_len*sizeof(double));
 

    GRIB_CHECK(grib_get_double_array(h,"values",values,&values_len),0);

    for (int i = 0; i < values_len; i++)
      values[i] = values[i] * 0.01;

    GRIB_CHECK(grib_set_double_array(h,"values",values,values_len),0);
 
    GRIB_CHECK(grib_write_message(h, argv[2], "w"), 0);
 
    free(values);
    grib_handle_delete(h);
 
    return 0;
}



