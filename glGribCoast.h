#pragma once

#include "glGribOptions.h"
#include "glGribLines.h"

class glGribCoast : public glGribLines
{
public:
  void setup (const glGribOptionsCoast &);
  const glGribOptionsCoast & getOptions () const { return opts; }
  float getScale () const override { return opts.lines.scale; }
private:
  glGribOptionsCoast opts;
};


