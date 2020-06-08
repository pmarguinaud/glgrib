#pragma once

#include "glGribOptionsBase.h"
#include "glGribOptionsLines.h"

namespace glGrib
{

class OptionsRivers : public OptionsBase
{
public:
  DEFINE
  {
    DESC (on, Display rivers);
    INCLUDE (lines);
  }
  bool on = false;
  OptionsLines lines = OptionsLines ("coastlines/gshhg/WDBII_bin/wdb_rivers_f.b", "gshhg");
};

};


