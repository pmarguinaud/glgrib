#pragma once

#include "glGribOptions.h"
#include "glGribLines.h"

class glGribBorder : public glGribLines
{
public:
  void setup (const glgrib_options_border &);
  const glgrib_options_border & getOptions () const { return opts; }
  float getScale () const override { return opts.lines.scale; }
private:
  glgrib_options_border opts;
};


