#include "glGribDepartements.h"

void glGrib::Departements::setup (const glGrib::OptionsDepartements & o)
{
  opts = o;
  glGrib::Lines::setup (opts.lines);
}
