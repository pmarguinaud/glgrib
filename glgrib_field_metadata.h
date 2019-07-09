#ifndef _GLGRIB_FIELD_METADATA
#define _GLGRIB_FIELD_METADATA

class glgrib_field_metadata
{
public:
  std::string CLNOMA = ""; // FA name
  float valmis, valmin, valmax; // Field min/max, missing values;
  long int discipline = 255, parameterCategory = 255, parameterNumber = 255; // GRIB2 metadata
  typedef struct 
  {
    long int year   = 0;
    long int month  = 0;
    long int day    = 0;
    long int hour   = 0;
    long int minute = 0;
    long int second = 0;
  } date_t;
  date_t base, term;
  long int indicatorOfUnitOfTimeRange = 255;
  long int forecastTime = 0;
  double forecastTerm = 0;
};

#endif
