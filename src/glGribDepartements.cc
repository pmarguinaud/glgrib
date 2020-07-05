#include "glGribDepartements.h"

namespace glGrib
{

void Departements::setup (const OptionsDepartements & o)
{
  if (! o.on)
    return;

  opts = o;
  Lines::setup (opts.lines);
}

}
