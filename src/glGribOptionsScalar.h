#pragma once

#include "glGribOptionsBase.h"

namespace glGrib
{

class OptionsScalar : public OptionsBase
{
public:
  DEFINE
  {
    DESC (smooth.on,    Smooth scalar fields);
    DESC (wireframe.on, Display field as wireframe);
    DESC (points.on,    Display field as points);
    DESC (points.size.value,  Field point size);
    DESC (points.size.variable.on,  Variable field point size);
    DESC (points.size.factor.on,  Apply scale factor to point size);
    DESC (pack.bits,    Number of bytes used to pack field);
    DESC (discrete.on,  Plot as a discrete field);
    DESC (discrete.missing_color, Color for missing values);
  }

  struct 
  {
    bool on = false;
  } smooth;
  struct 
  {
    bool on = false;
  } wireframe;
  struct 
  {
    bool on = false;
    struct
    {
      float value = 1.0f;
      struct
      {
        bool on = false;
      } variable;
      struct
      {
        bool on = true;
      } factor;
    } size;
  } points;
  struct
  {
    int bits = 8;
  } pack;
  struct
  {
    bool on = false;
    OptionColor missing_color = OptionColor (0, 0, 0, 0);
  } discrete;
};

};


