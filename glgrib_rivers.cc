#include "glgrib_rivers.h"

void glgrib_rivers::setup (const glgrib_options_rivers & o)
{
  opts = o;
  glgrib_lines::setup (opts.lines);
}
