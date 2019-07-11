#ifndef _GLGRIB_FIELD_METADATA
#define _GLGRIB_FIELD_METADATA

#include "glgrib_options.h"

class glgrib_field_metadata
{
public:
  std::string CLNOMA = ""; // FA name
  float valmis, valmin, valmax; // Field min/max, missing values;
  long int discipline = 255, parameterCategory = 255, parameterNumber = 255; // GRIB2 metadata
  glgrib_option_date base, term;
  long int indicatorOfUnitOfTimeRange = 255;
  long int forecastTime = 0;
  double forecastTerm = 0;
};

#endif
