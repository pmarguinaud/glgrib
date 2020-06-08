#pragma once

#include "glGribOptionsBase.h"
#include "glGribOptionsFont.h"
#include "glGribOptionsTicksSide.h"

namespace glGrib
{

class OptionsTicks : public OptionsBase
{
public:
  DEFINE
  {
    DESC (visible.on,              Ticks are visible);
    DESC (lines.on,                Display ticks);
    DESC (lines.color,             Tick color);
    DESC (lines.length,            Tick length);
    DESC (lines.width,             Tick width);
    DESC (lines.kind,              Tick kind);
    DESC (labels.on,               Display tick labels);
    DESC (labels.format,           Format for tick labels);
    INCLUDE (labels.font);
    DESC (frame.on,                Enable frame);
    DESC (frame.width,             Frame width);
    DESC (frame.color,             Frame color);
    INCLUDE (N);
    INCLUDE (S);
    INCLUDE (W);
    INCLUDE (E);
  }
  struct
  {
    bool on = true;
  } visible;

  struct 
  {
    bool on = false;
    OptionsFont font = OptionsFont (0.02f);
    std::string format = "%+06.2f";
  } labels;
  struct
  {
    bool on = false;
    float width = 0.01f;
    OptionColor color = OptionColor (255,   0,   0);
  } frame;
  struct
  {
    bool on = false;
    OptionColor color = OptionColor (255, 255, 255);
    float length = 0.025f;
    float width  = 0.010f;
    int kind     = 0;
  } lines;
  OptionsTicksSide N, S, W, E;
};

};


