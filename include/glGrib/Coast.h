#pragma once

#include "glGrib/Options.h"
#include "glGrib/Lines.h"

namespace glGrib
{

class Coast : public Lines
{
public:
  void setup (const OptionsCoast &);
  const OptionsCoast & getOptions () const { return opts; }
  float getScale () const override { return opts.lines.scale; }
private:
  OptionsCoast opts;
};



}
