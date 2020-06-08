#pragma once

#include "glGribOptionsDepartements.h"
#include "glGribLines.h"

namespace glGrib
{

class Departements : public Lines
{
public:
  void setup (const OptionsDepartements &);
  const OptionsDepartements & getOptions () const { return opts; }
  float getScale () const override { return opts.lines.scale; }
private:
  OptionsDepartements opts;
};



}
