#pragma once

#include "glGribOptionsBase.h"
#include "glGribOptionsPoints.h"

namespace glGrib
{

class OptionsGeoPoints : public OptionsBase
{
public:
  DEFINE 
  {
    DESC (on, Display geopoints);
    DESC (time.on, Make points appear with time);
    DESC (time.rate, Number of points to add at each new frame);
    INCLUDE (points);
    DESC (path, Path);
    DESC (lon, Longitude field name);
    DESC (lat, Latitude field name);
    DESC (val, Value field name);
  }
  bool on = false;
  struct
  {
    bool on = false;
    int rate = 1;
  } time;
  OptionsPoints points;
  std::string path;
  std::string lon, lat, val;
};

};


