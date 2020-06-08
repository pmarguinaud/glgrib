#pragma once

#include "glGribOptionsBase.h"
#include "glGribOptionsDate.h"
#include "glGribOptionsImage.h"
#include "glGribOptionsInterpolation.h"
#include "glGribOptionsLight.h"
#include "glGribOptionsText.h"
#include "glGribOptionsTitle.h"
#include "glGribOptionsTravelling.h"

namespace glGrib
{

class OptionsScene : public OptionsBase
{
public:
  DEFINE
  {
    DESC (lon_at_hour,         Set longitude at solar time);
    DESC (rotate_earth.on,     Make earth rotate);
    DESC (rotate_earth.rate,   Rate of rotation : angle/frame);
    INCLUDE (light);
    INCLUDE (travelling);
    INCLUDE (interpolation);
    INCLUDE (text);
    INCLUDE (image);
    INCLUDE (date);
    INCLUDE (title);
    DESC (select.field, Rank of field to select);
    DESC (center.on, Center on first field);
  }
  struct
  {
    bool on  = false;
    float rate = 1.0f;
  } rotate_earth;
  float   lon_at_hour = -1.0f;
  OptionsLight light;  
  OptionsTravelling travelling;
  OptionsInterpolation interpolation;
  OptionsDate date;
  OptionsText text;
  OptionsImage image;
  OptionsTitle title;
  struct
  {
    int field = 0;
  } select;
  struct
  {
    bool on = false;
  } center;
};

};


