#pragma once

#include "glGribOptions.h"
#include "glGribLines.h"

class glGribRivers : public glGribLines
{
public:
  void setup (const glGribOptionsRivers &);
  const glGribOptionsRivers & getOptions () const { return opts; }
  float getScale () const override { return opts.lines.scale; }
private:
  glGribOptionsRivers opts;
};


