#pragma once

#include "glGribOptionsBase.h"
#include "glGribOptionsFont.h"

namespace glGrib
{

class OptionsMapscale : public OptionsBase
{
public:
  DEFINE
  {
    DESC (on, Activate mapscale);
    INCLUDE (font);
    DESC (position.xmin, Mapscale position);
    DESC (position.xmax, Mapscale position); 
    DESC (position.ymin, Mapscale position); 
    DESC (position.ymax, Mapscale position);
    DESC (color1, First color);
    DESC (color2, Second color);
  }
  bool on = false;
  OptionsFont font = OptionsFont ("fonts/16.bmp", 0.02f);
  OptionColor color1 = OptionColor (255, 255, 255);
  OptionColor color2 = OptionColor ( 80,  80,  80);
  struct position
  {
    float xmin = 0.05;
    float xmax = 0.25; 
    float ymin = 0.05; 
    float ymax = 0.07;
  } position;
};

};


