#pragma once

#include "glGribOptionsBase.h"

namespace glGrib
{

class OptionsGeometry : public OptionsBase
{
public:
  DEFINE
  {
    DESC (triangle_strip.on,  Enable use of triangle strip);
    DESC (check.on,           Check geometry);
    DESC (height.on,          Enable height);
    DESC (height.path,        Height field path);
    DESC (height.scale,       Scale to apply to height field);
    DESC (frame.on,           Draw frame around domain);
    DESC (frame.color1,       First frame color);
    DESC (frame.color2,       Second frame color);
    DESC (frame.width,        Frame width);
    DESC (frame.dlon,         Longitude interval);
    DESC (frame.dlat,         Latitude interval);
    DESC (gencoords.on,       Generate coordinates on GPU when possible);
    DESC (gaussian.fit.on,    Fit Gaussian latitude retrieval);
    DESC (gaussian.apply_norm_scale.on, Apply norm scaling);
  }
  struct
  {
    bool on = false;
  } gencoords;
  struct
  {
    bool on = true;
  } triangle_strip;
  struct
  {
    bool on = false;
  } check;
  struct
  {
    bool on = false;
    std::string path = "";
    float scale = 0.05;
  } height;
  struct 
  {
    bool on = false;
    OptionColor color1 = OptionColor (255, 255, 255);
    OptionColor color2 = OptionColor ( 80,  80,  80);
    float width = 0.0f;
    float dlon = 10.0f;
    float dlat = 10.0f;
  } frame;
  struct
  {
    struct
    {
      bool on = false;
    } fit;
    struct
    {
      bool on = true;
    } apply_norm_scale;
  } gaussian;
};

};


