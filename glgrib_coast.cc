#include "glgrib_coast.h"

void glgrib_coast::init (const glgrib_options_coast & o)
{
  opts = o;
  glgrib_lines::init (opts.lines);
}
