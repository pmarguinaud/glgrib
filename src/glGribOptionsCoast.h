#pragma once

#include "glGribOptionsBase.h"
#include "glGribOptionsLines.h"

namespace glGrib
{

class OptionsCoast : public OptionsBase
{
public:
  DEFINE
  {
    DESC (on, Display coastlines);
    INCLUDE (lines);
    DESC (lakes.on, Display lakes coastlines);
  }
  bool on = false;
  struct
  {
    bool on = false;
  } lakes;
  OptionsLines lines = OptionsLines ("coastlines/gshhg/GSHHS_bin/gshhs_h.b", "gshhg");
};

};


