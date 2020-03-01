#include "glGribDepartements.h"

void glGribDepartements::setup (const glgrib_options_departements & o)
{
  opts = o;
  glGribLines::setup (opts.lines);
}
