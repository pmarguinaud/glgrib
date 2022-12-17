#pragma once

#include "glGrib/Options.h"
#include "glGrib/Lines.h"

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
