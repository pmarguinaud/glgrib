#pragma once

#include "glGribOptionsBase.h"

namespace glGrib
{

class OptionsIsofill : public OptionsBase
{
public:
  static float defaultMin () { return std::numeric_limits<float>::max (); }
  static float defaultMax () { return std::numeric_limits<float>::min (); }
  DEFINE
  {
    DESC (number,        Number of levels);
    DESC (levels,        List of levels);
    DESC (min,           Min level);
    DESC (max,           Max level);
  }
  int number = 10;
  std::vector<float> levels;
  float min = defaultMin ();
  float max = defaultMax ();
};

};


