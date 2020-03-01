#include "glGribCoast.h"

void glGribCoast::setup (const glgrib_options_coast & o)
{
  opts = o;
  if (opts.lakes.on)
    glGribLines::setup (opts.lines, {0xff, 0xff, 0xff, 0xff}, {1, 2, 3, 4});
  else
    glGribLines::setup (opts.lines, {0xff}, {1});
}
