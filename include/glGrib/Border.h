#pragma once

#include "glGrib/Options.h"
#include "glGrib/Lines.h"

namespace glGrib
{

class Border : public Lines
{
public:
  void setup (const OptionsBorder &);
  const OptionsBorder & getOptions () const { return opts; }
  float getScale () const override { return opts.lines.scale; }
private:
  OptionsBorder opts;
};



}
