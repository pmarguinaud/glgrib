#pragma once

#include "glGribOptionsBase.h"

namespace glGrib
{

class OptionsPalette : public OptionsBase
{
public:
  static float defaultMin () { return std::numeric_limits<float>::max (); }
  static float defaultMax () { return std::numeric_limits<float>::min (); }
  OptionsPalette () {}
  OptionsPalette (const std::string & n) : name (n) {}
  DEFINE
  {
    DESC (name,        Palette name);                              
    DESC (min,         Palette min value);                              
    DESC (max,         Palette max value);                              
    DESC (values,      Palette values);
    DESC (colors,      Palette colors);
    DESC (ncolors,     Number of colors);
    DESC (linear.on,   Linear colorbar display);
    DESC (scale,       Scale for colorbar figures);
    DESC (offset,      Offset for colorbar figures);
    DESC (rainbow.on,  Generate rainbow palette);
    DESC (rainbow.direct.on,  Direct/indirect rainbow);
    DESC (generate.on,        Generate values);
    DESC (generate.levels,    Number of values to generate);
    DESC (fixed.on,           Fixed palette);
  }
  string name = "default";
  float min = defaultMin ();
  float max = defaultMax ();
  std::vector<float> values;
  std::vector<OptionColor> colors;
  int ncolors = 256;
  struct
  {
    bool on = false;
  } linear;
  struct
  {
    bool on = false;
    struct
    {
      bool on = false;
    } direct;
  } rainbow;
  struct
  {
    bool on = false;
    int levels = 10;
  } generate;
  float scale = 1.0f;
  float offset = 0.0f;
  struct
  {
    bool on = false;
  } fixed;
};

};


