#pragma once

#include "glGribOptionsBase.h"
#include "glGribOptionsFont.h"

namespace glGrib
{

class OptionsDate : public OptionsBase
{
public:
  DEFINE
  {
    DESC (on, Display date);
    INCLUDE (font);
  }
  bool on = false;
  OptionsFont font;
};

};


