#pragma once

#include "glGribOptionsBase.h"
#include "glGribOptionsLines.h"

namespace glGrib
{

class OptionsBorder : public OptionsBase
{
public:
  DEFINE
  {
    DESC (on, Display borders);
    DESC (countries.on, Display countries);
    DESC (regions.on, Display regions);
    DESC (sea.on, Display sea borders);
    INCLUDE (lines);
  }
  bool on = false;
  struct
  {
    bool on = true;
  } countries;
  struct
  {
    bool on = false;
  } regions;
  struct
  {
    bool on = false;
  } sea;
  OptionsLines lines = OptionsLines ("coastlines/gshhg/WDBII_bin/wdb_borders_f.b", "gshhg");
};

};


