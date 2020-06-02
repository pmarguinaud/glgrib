#include "glGribRivers.h"

void glGrib::Rivers::setup (const glGrib::OptionsRivers & o)
{
  if (! o.on)
    return;

  opts = o;
  glGrib::Lines::setup (opts.lines);
}
