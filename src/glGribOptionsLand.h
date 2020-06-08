#pragma once

#include "glGribOptionsBase.h"
#include "glGribOptionsLandLayer.h"

namespace glGrib
{

class OptionsLand : public OptionsBase
{
public:
  DEFINE
  {
    DESC (on, Enable land);
    DESC (visible.on,  Land is visible);
    INCLUDE   (layers[0]);
    INCLUDE_H (layers[1]);
    INCLUDE_H (layers[2]);
    INCLUDE_H (layers[3]);
  }
  
  bool on = false;

  struct
  {
    bool on = true;
  } visible;

  std::vector<OptionsLandLayer> layers = 
  {
    OptionsLandLayer ("coastlines/shp/GSHHS_c_L1.shp", 1.000f, OptionColor ("#ffe2ab")),
    OptionsLandLayer ("coastlines/shp/GSHHS_c_L2.shp", 1.001f, OptionColor ("#0000ff")),
    OptionsLandLayer ("coastlines/shp/GSHHS_c_L3.shp", 1.002f, OptionColor ("#ffe2ab")),
    OptionsLandLayer ("coastlines/shp/GSHHS_c_L5.shp", 1.000f, OptionColor ("#ffe2ab")) 
  };

};

};


