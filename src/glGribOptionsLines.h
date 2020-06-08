#pragma once

#include "glGribOptionsBase.h"

namespace glGrib
{

class OptionsLines : public OptionsBase
{
public:
  OptionsLines () {}
  OptionsLines (const std::string & p, const string & f) : path (p), format (f) {}
  DEFINE
  {
    DESC (visible.on,         Lines are visible);
    DESC (path,               Path to lines file);
    DESC (color,              Coastlines color);
    DESC (scale,              Coastlines scale);
    DESC (format,             Input format);
    DESC (selector,           Selector);
    DESC (latmin,             Minimum latitude);
    DESC (latmax,             Maximum latitude);
    DESC (lonmin,             Minimum longitude);
    DESC (lonmax,             Maximum longitude);
  }
  struct
  {
    bool on = true;
  } visible;
  std::string selector;
  string path;
  string format = "gshhg";
  OptionColor color;
  float scale = 1.005;
  float latmin = 0.0f, latmax = 0.0f, lonmin = 0.0f, lonmax = 0.0f;
};

};


