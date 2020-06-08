#pragma once

#include "glGribOptionsBase.h"

namespace glGrib
{

class OptionsLandLayer : public OptionsBase
{
public:
  OptionsLandLayer (const std::string & _path, float _scale, const OptionColor & _color)
                           :  path (_path), scale (_scale), color (_color) {}
  DEFINE
  {
    DESC (selector,           Shape selection);
    DESC (path,               Path to coastlines);
    DESC (subdivision.angle,  Angle max for subdivision);
    DESC (subdivision.on,     Enable subdivision);
    DESC (on,                 Enable);
    DESC (scale,              Scale);
    DESC (color,              Land color);
    DESC_H (debug.on,         Debug);
  }
  std::string selector = "";
  std::string path     = "coastlines/shp/GSHHS_c_L1.shp";
  float scale          = 1.0f;
  struct
  {
    float angle        = 1.0f;
    bool on            = true;
  } subdivision;
  OptionColor color = OptionColor ("#ffe2ab");
  struct
  {
    bool on            = false;
  } debug;
  bool on              = true;
};

};


