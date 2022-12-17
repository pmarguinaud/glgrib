#pragma once

#include "glGrib/Points.h"
#include "glGrib/View.h"
#include "glGrib/Options.h"
#include "glGrib/String.h"
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
    String3D<1,0> labels;
  } d;
};


}
