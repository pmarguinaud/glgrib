#pragma once

#include "glGribOptions.h"
#include "glGribLines.h"

class glGribRivers : public glGribLines
{
public:
  void setup (const glgrib_options_rivers &);
  const glgrib_options_rivers & getOptions () const { return opts; }
  float getScale () const override { return opts.lines.scale; }
private:
  glgrib_options_rivers opts;
};


