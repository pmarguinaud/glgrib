#pragma once

#include "glGribOptionsBase.h"
#include "glGribOptionsFont.h"
#include "glGribOptionsPoints.h"

namespace glGrib
{

class OptionsCities : public OptionsBase
{
public:
  DEFINE 
  {
    DESC (on, Display cities);
    INCLUDE (points);
    INCLUDE (labels.font);
    DESC (labels.on, Enable city names display);
  }
  bool on = false;
  OptionsPoints points;
  struct
  {
    OptionsFont font;
    bool on = false;
  } labels;
};

};


