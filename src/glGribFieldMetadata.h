#pragma once

#include "glGribOptions.h"
#include <string>

namespace glGrib
{

class FieldMetadata
{
public:
  std::string CLNOMA = ""; // FA name
  std::string shortName;
  float valmis, valmin, valmax; // Field min/max, missing values;
  long int discipline = 255, parameterCategory = 255, parameterNumber = 255; // GRIB2 metadata
  OptionDate base, term;
  long int indicatorOfUnitOfTimeRange = 255;
  long int forecastTime = 0;
  double forecastTerm = 0;
  const std::string & getName () const 
  {
    if (name == "") 
      {
        if (CLNOMA != "") 
          name = CLNOMA; 
        else if (shortName != "")
          name = shortName;
        else
          {
            char tmp[64];
            sprintf (tmp, "%3ld/%3ld/%3ld", discipline, parameterCategory, parameterNumber);
            name = std::string (tmp);
          }
      } 
    return name; 
  }
private:
  mutable std::string name;
};


}
