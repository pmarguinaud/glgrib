#pragma once

#include "glGribOptionsBase.h"

namespace glGrib
{

class OptionsVector : public OptionsBase
{
public:
  DEFINE
  {
    DESC (arrow.on,        Show arrows);                
    DESC (norm.on,         Show norm field);            
    DESC (arrow.color,     Color for arrows);
    DESC (density,         Vector density);
    DESC (scale,           Vector scale);
    DESC (arrow.head_size, Vector head size);
    DESC (arrow.kind,      Arrow kind);
    DESC (arrow.fill.on,   Fill arrow);
    DESC (arrow.fixed.on,  Fixed sized arrow);
    DESC (arrow.min,       Vector min value);
    DESC (barb.on,         Enable barbs);
    DESC (barb.color,      Barbs color);
    DESC (barb.angle,      Barbs angle);
    DESC (barb.length,     Barbs length);
    DESC (barb.dleng,      Barbs length);
    DESC (barb.levels,     Barbs levels);
    DESC (barb.circle.level,   Value under which we draw a circle);
    DESC (barb.pennant.levels, Pennants levels);
  }
  struct
  {
    bool on = true;
    float head_size = 0.1f;
    int kind = 0;
    struct
    {
      bool on = false;
    } fill;
    OptionColor color;
    struct
    {
      bool on = false;
    } fixed;
    float min = 0.0f;
  } arrow;
  struct
  {
    bool on = false;
    OptionColor color;
    float angle  = 60.0;
    float length = 0.25;
    float dleng  = 0.1;
    std::vector<float> levels = {0., 5., 10., 15., 20., 25.};
    struct
    {
      float level = 1.0f;
    } circle;
    struct
    {
      std::vector<float> levels = {0., 25., 50., 75.};
    } pennant;
  } barb;
  struct
  {
    bool on = true;
  } norm;
  float density = 50.0f;
  float scale = 1.0f;
};

};


