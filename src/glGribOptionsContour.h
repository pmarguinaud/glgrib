#pragma once

#include "glGribOptionsBase.h"
#include "glGribOptionsFont.h"

namespace glGrib
{

class OptionsContour : public OptionsBase
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
    DESC (widths,        List of widths);
    DESC (patterns,      List of dash patterns);
    DESC (lengths,       List of dash lengths);
    DESC (labels.on,     Enable labels);
    INCLUDE (labels.font);
    DESC (labels.distmin, Minimal length in degrees for labelled lines);
    DESC (labels.format,  Format to print labels);
  }
  int number = 10;
  std::vector<float> levels;
  float min = defaultMin ();
  float max = defaultMax ();
  std::vector<float> widths;
  std::vector<std::string> patterns;
  std::vector<float> lengths;
  struct
  {
    bool on = false;
    OptionsFont font;
    float distmin = 3.0f;
    std::string format = "%12.2f";
  } labels;
};

};


