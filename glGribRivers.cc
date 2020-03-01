#include "glGribRivers.h"

void glGribRivers::setup (const glGribOptionsRivers & o)
{
  opts = o;
  glGribLines::setup (opts.lines);
}
