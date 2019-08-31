#include "glgrib_departements.h"

void glgrib_departements::init (const glgrib_options_departements & o)
{
  opts = o;
  glgrib_lines::init (opts.lines);
}
