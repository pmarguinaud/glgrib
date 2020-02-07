#pragma once

#include "glgrib_options.h"
#include "glgrib_lines.h"

class glgrib_departements : public glgrib_lines
{
public:
  void setup (const glgrib_options_departements &);
  const glgrib_options_departements & getOptions () const { return opts; }
  float getScale () const override { return opts.lines.scale; }
private:
  glgrib_options_departements opts;
};


