#include "glgrib_departements.h"

void glgrib_departements::setup (const glgrib_options_departements & o)
{
  opts = o;
  glgrib_lines::setup (opts.lines);
}
