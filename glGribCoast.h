#pragma once

#include "glGribOptions.h"
#include "glGribLines.h"

class glGribCoast : public glGribLines
{
public:
  void setup (const glgrib_options_coast &);
  const glgrib_options_coast & getOptions () const { return opts; }
  float getScale () const override { return opts.lines.scale; }
private:
  glgrib_options_coast opts;
};


