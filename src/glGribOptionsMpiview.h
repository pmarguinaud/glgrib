#pragma once

#include "glGribOptionsBase.h"

namespace glGrib
{

class OptionsMpiview : public OptionsBase
{
public:
  DEFINE
  { 
    DESC (path,  Path to MPI distribution field);
    DESC (on,    Enable MPI view);
    DESC (scale, Displacement scale);
  }

  std::vector<std::string> path;
  bool on = false;
  float scale = 0.1f;
};

};


