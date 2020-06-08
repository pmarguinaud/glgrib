#pragma once

#include "glGribOptionsBase.h"

namespace glGrib
{

class OptionsImage : public OptionsBase
{
public:
  DEFINE
  {
    DESC (on,            Enable image display);
    DESC (path,          Path to image);
    DESC (x0,            Left abscissa);
    DESC (y0,            Lower ordinate);
    DESC (x1,            Right abscissa);
    DESC (y1,            Upper ordinate);
    DESC (align,         Image alignment);
  }  
  bool on = false;
  std::string path = "";
  float x0 = 0.0, x1 = 1.0, y0 = 0.0, y1 = 1.0;
  std::string align;
};

};


