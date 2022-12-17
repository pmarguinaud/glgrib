#pragma once

#include "glGrib/Options.h"
#include "glGrib/Lines.h"

namespace glGrib
{

class Rivers : public Lines
{
public:
  void setup (const OptionsRivers &);
  const OptionsRivers & getOptions () const { return opts; }
  float getScale () const override { return opts.lines.scale; }
private:
  OptionsRivers opts;
};



}
