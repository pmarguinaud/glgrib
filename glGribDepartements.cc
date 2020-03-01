#include "glGribDepartements.h"

void glGribDepartements::setup (const glGribOptionsDepartements & o)
{
  opts = o;
  glGribLines::setup (opts.lines);
}
