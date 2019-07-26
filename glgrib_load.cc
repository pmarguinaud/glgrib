#include "glgrib_load.h"
#include "glgrib_geometry.h"

#include <stdio.h>
#include <stdlib.h>
#include <eccodes.h>
#include <iostream>
#include <stdexcept>
#include <string>
#include <errno.h>
#include <time.h>


void glgrib_load (const std::vector<std::string> & file, float fslot, float ** val, 
                  glgrib_field_metadata * meta, int mult, int base)
{
  int islot = (int)fslot;

  if (fslot == (float)islot)
    return glgrib_load (file[mult*islot+base], val, meta);

  const std::string file1 = file[mult*(islot+0)+base];
  const std::string file2 = file[mult*(islot+1)+base];

  const_glgrib_geometry_ptr geom1 = glgrib_geometry_load (file1);
  const_glgrib_geometry_ptr geom2 = glgrib_geometry_load (file2);

  if (! geom1->isEqual (*geom2))
    {
      throw std::runtime_error (std::string ("Vector components have different geometries : ") 
                                + file1 + ", " + file2);
    }

  float * val1 = NULL, * val2 = NULL;
  glgrib_field_metadata meta1, meta2;

  glgrib_load (file1, &val1, &meta1);
  glgrib_load (file2, &val2, &meta2);


  *meta = meta1;

  float alpha = fslot - (float)islot;
  meta->term = glgrib_option_date::interpolate (meta1.term, meta2.term, fslot - alpha);

}

void glgrib_load (const std::string & file, float ** val, glgrib_field_metadata * meta)
{
  FILE * in = fopen (file.c_str (), "r");

  if (in == NULL)
    {
      throw std::runtime_error (std::string ("Cannot open ") + file + ": " + strerror (errno));
    }

  *val = NULL;

  int err = 0;
  codes_handle * h = codes_handle_new_from_file (0, in, PRODUCT_GRIB, &err);
  fclose (in);

  if (h == NULL)
    throw std::runtime_error (std::string ("`") + file + "' does not contain GRIB data");

  size_t v_len = 0;
  codes_get_size (h, "values", &v_len);

  double vmis, vmin, vmax;
  codes_get_double (h, "missingValue", &vmis);
  codes_get_double (h, "minimum",      &vmin);
  codes_get_double (h, "maximum",      &vmax);
  double * v = (double *)malloc (sizeof (double) * v_len);
  codes_get_double_array (h, "values", v, &v_len);

  *val = new float [v_len];
  for (int i = 0; i < v_len; i++)
    (*val)[i] = v[i];

  free (v);

  meta->valmis = vmis;
  meta->valmin = vmin;
  meta->valmax = vmax;

  meta->CLNOMA = "";
  if (codes_is_defined (h, "CLNOMA"))
    {
      size_t len;
      codes_get_length (h, "CLNOMA", &len);
      char CLNOMA[len+1];
      codes_get_string (h, "CLNOMA", CLNOMA, &len);
      meta->CLNOMA = std::string (CLNOMA);
    }

#define CODES_GET(attr) \
  do {                                             \
    meta->attr = 255;                              \
    if (codes_is_defined (h, #attr))               \
      codes_get_long (h, #attr, &meta->attr);      \
  } while (0)

  CODES_GET (discipline);
  CODES_GET (parameterCategory);
  CODES_GET (parameterNumber);
  CODES_GET (indicatorOfUnitOfTimeRange);
  CODES_GET (forecastTime);
#undef CODES_GET

#define CODES_GET(attr) \
  do {                                             \
    meta->base.attr = 255;                         \
    if (codes_is_defined (h, #attr))               \
      codes_get_long (h, #attr, &meta->base.attr); \
  } while (0)

  CODES_GET (year);
  CODES_GET (month);
  CODES_GET (day);
  CODES_GET (hour);
  CODES_GET (minute);
  CODES_GET (second);

#undef CODES_GET

  switch (meta->indicatorOfUnitOfTimeRange)
    {
      case  0:        // m Minute 
        meta->forecastTerm = meta->forecastTime * 60.0;
        break;
      case  1:        // h Hour 
        meta->forecastTerm = meta->forecastTime *        3600.0;
        break;
      case  2:        // D Day 
        meta->forecastTerm = meta->forecastTime * 24.0 * 3600.0;
        break;
      case 10:        // 3h 3 hours 
        meta->forecastTerm = meta->forecastTime *  3.0 * 3600.0;
        break;
      case 11:        // 6h 6 hours 
        meta->forecastTerm = meta->forecastTime *  6.0 * 3600.0;
        break;
      case 12:        // 12h 12 hours 
        meta->forecastTerm = meta->forecastTime * 12.0 * 3600.0;
        break;
      case 13:        // s Second 
        meta->forecastTerm = meta->forecastTime;
        break;
      case 255:
        meta->forecastTerm = 0;
        break;
      default:
        throw std::runtime_error (std::string ("Unexpected indicatorOfUnitOfTimeRange found in `") + file + std::string ("'"));
    }

  meta->term = glgrib_option_date::date_from_t (glgrib_option_date::t_from_date (meta->base) + meta->forecastTerm);

  if (false)
  std::cout 
   << " " << meta->term.year  
   << " " << meta->term.month 
   << " " << meta->term.day   
   << " " << meta->term.hour  
   << " " << meta->term.minute
   << " " << meta->term.second
   << " " << std::endl;

  codes_handle_delete (h);

}

