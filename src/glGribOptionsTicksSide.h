#pragma once

#include "glGribOptionsBase.h"

namespace glGrib
{

class OptionsTicksSide : public OptionsBase
{
public:
  DEFINE
  {
    DESC (on, Enable side);
    DESC (dlon, Longitude interval);
    DESC (dlat, Latitude interval);
    DESC (nswe.on, North/South/West/East labels instead of +/-);
  }
  bool on = true;
  float dlon = 10.0f;
  float dlat = 10.0f;
  struct
  {
    bool on = false;
  } nswe;
};

};


