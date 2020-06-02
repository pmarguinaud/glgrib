#include "glGribDepartements.h"

void glGrib::Departements::setup (const glGrib::OptionsDepartements & o)
{
  if (! o.on)
    return;

  opts = o;
  glGrib::Lines::setup (opts.lines);
}
