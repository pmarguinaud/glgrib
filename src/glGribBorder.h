#pragma once

#include "glGribOptions.h"
#include "glGribLines.h"

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
