#include "glgrib_load.h"

#include <stdio.h>
#include <stdlib.h>
#include <eccodes.h>
#include <iostream>
#include <stdexcept>
#include <string>
#include <errno.h>
#include <time.h>


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


  struct tm t;
  t.tm_sec    = meta->base.second;
  t.tm_min    = meta->base.minute;
  t.tm_hour   = meta->base.hour;
  t.tm_mday   = meta->base.day;
  t.tm_mon    = meta->base.month - 1;  
  t.tm_year   = meta->base.year - 1900; 

  time_t time = mktime (&t) + meta->forecastTerm;

  gmtime_r (&time, &t);

  meta->term.second = t.tm_sec;
  meta->term.minute = t.tm_min;
  meta->term.hour   = t.tm_hour;
  meta->term.day    = t.tm_mday;
  meta->term.month  = t.tm_mon + 1;
  meta->term.year   = t.tm_year + 1900;

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

