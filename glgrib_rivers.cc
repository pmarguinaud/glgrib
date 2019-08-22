#include "glgrib_rivers.h"

void glgrib_rivers::init (const glgrib_options_rivers & o)
{
  opts = o;
  glgrib_lines::init (opts.lines);
}
