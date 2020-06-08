#pragma once

#include "glGribOptionsBase.h"
#include "glGribOptionsFont.h"

namespace glGrib
{

class OptionsText : public OptionsBase
{
public:
  DEFINE
  {
    DESC (on,             Enable text);
    DESC (s,              Strings to be displayed);
    DESC (x,              Coordinates of strings);
    DESC (y,              Coordinates of strings);
    DESC (a,              Text alignment);
    INCLUDE (font);
  }
  bool on = false;
  std::vector<std::string> s;
  std::vector<float> x;
  std::vector<float> y;
  std::vector<std::string> a;
  OptionsFont font;
};

};


