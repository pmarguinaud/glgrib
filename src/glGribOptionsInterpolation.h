#pragma once

#include "glGribOptionsBase.h"

namespace glGrib
{

class OptionsInterpolation : public OptionsBase
{
public:
  DEFINE
  {
    DESC (on,             Enable interpolation);
    DESC (frames,         Number of frames);
  }
  bool on = false;
  int frames = 10;
};

};


