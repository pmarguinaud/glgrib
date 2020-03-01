#pragma once

#include "glGribOptions.h"
#include "glGribLines.h"

class glGribDepartements : public glGribLines
{
public:
  void setup (const glgrib_options_departements &);
  const glgrib_options_departements & getOptions () const { return opts; }
  float getScale () const override { return opts.lines.scale; }
private:
  glgrib_options_departements opts;
};


