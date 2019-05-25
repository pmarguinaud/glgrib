#ifndef _GLGRIB_FIELD_METADATA
#define _GLGRIB_FIELD_METADATA

class glgrib_field_metadata
{
public:
  std::string CLNOMA = ""; // FA name
  float valmis, valmin, valmax; // Field min/max, missing values;
  long int discipline = 255, parameterCategory = 255, parameterNumber = 255; // GRIB2 metadata
};

#endif
