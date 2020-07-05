#include "glGribRivers.h"

namespace glGrib
{

void Rivers::setup (const OptionsRivers & o)
{
  if (! o.on)
    return;

  opts = o;
  Lines::setup (opts.lines);
}

}
