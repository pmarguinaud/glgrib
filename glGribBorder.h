#pragma once

#include "glGribOptions.h"
#include "glGribLines.h"

class glGribBorder : public glGribLines
{
public:
  void setup (const glGribOptionsBorder &);
  const glGribOptionsBorder & getOptions () const { return opts; }
  float getScale () const override { return opts.lines.scale; }
private:
  glGribOptionsBorder opts;
};


