#pragma once

#include "glGribOptionsBase.h"
#include "glGribOptionsPalette.h"

namespace glGrib
{

class OptionsPoints : public OptionsBase
{
public:
  DEFINE 
  {
    DESC (visible.on,  Points are visible);
    DESC (scale, Scale);
    DESC (size.variable.on, Enable variable point size);
    DESC (size.value, Point size);
    INCLUDE (palette);
    DESC (color, Point color);
  }
  struct
  {
    bool on = true;
  } visible;
  OptionsPalette palette = OptionsPalette ("none");
  OptionColor color;
  float scale = 1.0f;
  struct
  {
    float value = 1.0f;
    struct
    {
       bool on = false;
    } variable;
  } size;
};

};


