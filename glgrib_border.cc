#include "glgrib_border.h"

void glgrib_border::init (const glgrib_options_border & o)
{
  opts = o;
  glgrib_lines::init (opts.lines);
}
