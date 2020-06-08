#pragma once

#include "glGribOptionsBase.h"
#include "glGribOptionsLines.h"

namespace glGrib
{

class OptionsDepartements : public OptionsBase
{
public:
  DEFINE
  {
    DESC (on, Display departements);
    INCLUDE (lines);
  }
  bool on = false;
  OptionsLines lines = OptionsLines ("coastlines/departements/departements-20180101.shp", "shapeline");
};

};


