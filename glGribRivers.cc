#include "glGribRivers.h"

void glGribRivers::setup (const glgrib_options_rivers & o)
{
  opts = o;
  glGribLines::setup (opts.lines);
}
