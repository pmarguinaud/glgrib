#pragma once

#include "glGribOptionsBase.h"

namespace glGrib
{

class OptionsLandscapePosition : public OptionsBase
{
public:
  DEFINE
  {
    DESC (lon1,       First longitude of display);
    DESC (lon2,       Last longitude of display);
    DESC (lat1,       First latitude of display);
    DESC (lat2,       Last latitude of display);
  }
  float lon1 = -180.0f, lon2 = +180.0f, lat1 = -90.0f, lat2 = +90.0f;
};

};


