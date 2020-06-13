#pragma once

#include "glGribPoints.h"
#include "glGribView.h"
#include "glGribOptions.h"
#include "glGribString.h"
#include <string>
#include <vector>


namespace glGrib
{

class Cities : public Points
{
public:
  void render (const View &, const OptionsLight &) const;
  void setup (const OptionsCities &);
  const OptionsCities & getOptions () const { return opts; }
  float getScale () const override { return opts.points.scale; }
private:
  OptionsCities opts;
  struct
  {
    String3D<true,false> labels;
  } d;
};


}
