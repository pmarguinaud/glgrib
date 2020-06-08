#pragma once

#include "glGribOptionsBase.h"

namespace glGrib
{

class OptionsLight : public OptionsBase
{
public:
  DEFINE
  {
    DESC (date_from_grib.on, Calculate light position from GRIB date);
    DESC (on,                Enable light);
    DESC (lon,               Light longitude);
    DESC (lat,               Light latitude);
    DESC (rotate.on,         Make sunlight move);
    DESC (rotate.rate,       Rate of rotation : angle/frame);
    DESC (date,              Date for sunlight position);
    DESC (night,             Fraction of light during for night);
  }
  OptionDate date;
  bool   on  = false;
  struct
  {
    bool on = false;
  } date_from_grib;
  struct
  {
    bool on = false;
    float rate = 1.0f;
  } rotate;
  float  lon  = 0.0f;
  float  lat  = 0.0f;
  float  night = 0.1f;
};

};


