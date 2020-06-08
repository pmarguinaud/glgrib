#pragma once

#include "glGribOptionsBase.h"
#include "glGribOptionsFont.h"

namespace glGrib
{

class OptionsGrid : public OptionsBase
{
public:
  DEFINE
  {
    DESC (visible.on,        Grid is visible);
    DESC (on,                Display grid);
    DESC (resolution,        Grid resolution);
    DESC (interval,          Interval between non dashed lines);
    DESC (dash_length,       Dash length in degrees);
    DESC (points,            Number of points along a parallel);
    DESC (color,             Grid color);
    DESC (scale,             Grid scale);
    DESC (labels.on,         Enable labels);
    DESC (labels.lon,        Longitude of latitude labels);
    DESC (labels.lat,        Latitude of longitude labels);
    DESC (labels.angle,      Angle of labels);
    INCLUDE (labels.font);
  }

  struct
  {
    bool on = true;
  } visible;
  int resolution = 9;
  int points = 200;
  int interval = 2;
  float dash_length = 4.0f;
  OptionColor color = OptionColor (0, 255, 0);
  bool on = false;
  float scale = 1.005;
  struct 
  {
    bool on = false;
    float lon = 0.0f, lat = 0.0f;
    float angle = 0.0f;
    OptionsFont font;
  } labels;
};

};


