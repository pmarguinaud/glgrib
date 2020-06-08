#pragma once

#include "glGribOptionsBase.h"
#include "glGribOptionsFont.h"

namespace glGrib
{

class OptionsColorbar : public OptionsBase
{
public:
  DEFINE
  {
    DESC (on, Activate colorbar);
    INCLUDE (font);
    DESC (levels.number, Colorbar number of levels);
    DESC (levels.values, Colorbar level values);
    DESC (format, Format (sprintf) use to display numbers);
    DESC (position.xmin, Colorbar position);
    DESC (position.xmax, Colorbar position); 
    DESC (position.ymin, Colorbar position); 
    DESC (position.ymax, Colorbar position);
  }
  bool on = false;
  OptionsFont font = OptionsFont ("fonts/16.bmp", 0.02f);
  std::string format = "%6.4g";
  struct
  {
    int number = 11;
    std::vector<float> values;
  } levels;
  struct position
  {
    float xmin = 0.08;
    float xmax = 0.18; 
    float ymin = 0.05; 
    float ymax = 0.95;
  } position;
};

};


