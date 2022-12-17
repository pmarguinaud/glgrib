#include "glGrib/Lines.h"
#include "glGrib/Program.h"
#include "glGrib/Resolve.h"
#include "glGrib/GSHHG.h"
#include "glGrib/ShapeLib.h"

#include <cmath>
#include <iostream>
#include <stdexcept>

namespace glGrib
{

void Lines::setup (const OptionsLines & o, 
                   const std::vector<unsigned int> & mask, 
                   const std::vector<unsigned int> & code)
{
  opts = o;

  std::vector <float> lonlat;
  std::vector <unsigned int> ind;

  if (opts.format == "gshhg")
     GSHHG::read (opts, &lonlat, &ind, mask, code);
  else
     ShapeLib::read (opts, &lonlat, &ind, opts.selector);

  Polygon::setup (lonlat, ind);

  setReady ();
}

void Lines::render (const View & view, const OptionsLight & light) const
{
  Program * program = Program::load ("MONO");
  program->use ();
  view.setMVP (program);
  program->set (light);
  program->set ("color0", opts.color);
  program->set ("do_alpha", 1);
  program->set ("scale", opts.scale);

  Polygon::render (view, light);

  view.delMVP (program);

}

}
