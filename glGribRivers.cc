#include "glGribRivers.h"

void glGrib::Rivers::setup (const glGrib::OptionsRivers & o)
{
  opts = o;
  glGrib::Lines::setup (opts.lines);
}
