#pragma once

#include "glGribOptionsBase.h"
#include "glGribOptionsFont.h"

namespace glGrib
{

class OptionsTitle : public OptionsBase
{
public:
  DEFINE
  {
    DESC (on, Enable title);
    DESC (x,  Coordinates);
    DESC (y,  Coordinates);
    DESC (a,  Alignment);
    INCLUDE (font);
    DESC (text, Title);
  }
  bool on = false;
  float x = 0.;
  float y = 1.;
  std::string a = "NW";
  std::string text = "";
  OptionsFont font;
};

};


