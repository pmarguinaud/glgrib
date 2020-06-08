#pragma once

#include "glGribOptionsBase.h"
#include "glGribOptionsPosition.h"

namespace glGrib
{

class OptionsTravelling : public OptionsBase
{
public:
  DEFINE
  {
    DESC (on,             Enable travelling); 
    DESC (frames,         Number of frames);
    INCLUDE (pos1);
    INCLUDE (pos2);
  }
  bool   on     = false;
  int    frames = 100;
  OptionsPosition pos1;
  OptionsPosition pos2;
};

};


