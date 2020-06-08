#pragma once

#include "glGribOptionsBase.h"

namespace glGrib
{

class OptionsFont : public OptionsBase
{
public:
  OptionsFont (const std::string & b, float s) : bitmap (b), scale (s) {}
  OptionsFont (float s) : scale (s) {}
  OptionsFont () {}
  DEFINE
  {
    DESC (bitmap,     Bitmap path);
    DESC (scale,      Bitmap scale);
    DESC (color.foreground, Foreground color);
    DESC (color.background, Background color);
  }
  std::string bitmap = "fonts/08.bmp";
  float scale = 0.05f;
  struct
  {
    OptionColor foreground = OptionColor (255, 255, 255, 255);
    OptionColor background = OptionColor (  0,   0,   0,   0);
  } color;
};

};


