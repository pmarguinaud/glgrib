#include "glGribCoast.h"

void glGrib::Coast::setup (const glGrib::OptionsCoast & o)
{
  opts = o;
  if (opts.lakes.on)
    glGrib::Lines::setup (opts.lines, {0xff, 0xff, 0xff, 0xff}, {1, 2, 3, 4});
  else
    glGrib::Lines::setup (opts.lines, {0xff}, {1});
}
