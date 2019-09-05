#include "glgrib_coast.h"

void glgrib_coast::setup (const glgrib_options_coast & o)
{
  opts = o;
  if (opts.lakes.on)
    glgrib_lines::setup (opts.lines, {0xff, 0xff, 0xff, 0xff}, {1, 2, 3, 4});
  else
    glgrib_lines::setup (opts.lines, {0xff}, {1});
}
