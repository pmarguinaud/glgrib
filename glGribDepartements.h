#pragma once

#include "glGribOptions.h"
#include "glGribLines.h"

class glGribDepartements : public glGribLines
{
public:
  void setup (const glGribOptionsDepartements &);
  const glGribOptionsDepartements & getOptions () const { return opts; }
  float getScale () const override { return opts.lines.scale; }
private:
  glGribOptionsDepartements opts;
};


