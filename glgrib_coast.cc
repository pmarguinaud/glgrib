#include "glgrib_coast.h"

void glgrib_coast::init (const glgrib_options_coast & o)
{
  opts = o;
  if (opts.lakes.on)
    glgrib_lines::init (opts.lines, {0xff, 0xff, 0xff, 0xff}, {1, 2, 3, 4});
  else
    glgrib_lines::init (opts.lines, {0xff}, {1});
}
