#pragma once

#include "glGribOptionsBase.h"

namespace glGrib
{

class OptionsStream : public OptionsBase
{
public:
  DEFINE
  {
    DESC (width,         Stream lines width);          
    DESC (density,       Stream lines density);          
    DESC (motion.on,     Enable stream lines motion);
    DESC (motion.timeaccel, Time acceleration);
    DESC (motion.nwaves,    Number of waves over 1000km);
  }
  float width = 0.0f;
  float density = 1.0f;
  struct
  {
    bool on = false;
    float timeaccel = 10000.0f;
    float nwaves = 10.f;
  } motion;
};

};


