#include "glGribLines.h"
#include "glGribProgram.h"
#include "glGribResolve.h"
#include "glGribGSHHG.h"
#include "glGribShapeLib.h"

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <stdexcept>

void glGrib::Lines::setup (const glGrib::OptionsLines & o, 
                          const std::vector<unsigned int> & mask, 
                          const std::vector<unsigned int> & code)
{

  opts = o;

  std::vector <float> lonlat;
  std::vector <unsigned int> ind;

  if (opts.format == "gshhg")
     glGrib::GSHHG::read (opts, &numberOfPoints, &numberOfLines, &lonlat, &ind, mask, code);
  else
     glGrib::ShapeLib::read (opts, &numberOfPoints, &numberOfLines, &lonlat, &ind, opts.selector);

  vertexbuffer = glGrib::OpenGLBufferPtr<float> (lonlat);
  elementbuffer = glGrib::OpenGLBufferPtr<unsigned int> (ind);

  setupVertexAttributes ();

  setReady ();
}

void glGrib::Lines::render (const glGrib::View & view, const glGrib::OptionsLight & light) const
{
  glGrib::Program * program = glGrib::Program::load ("MONO");
  program->use ();
  view.setMVP (program);
  program->set (light);
  program->set ("color0", opts.color);
  program->set ("do_alpha", 1);
  program->set ("scale", opts.scale);

  glGrib::Polygon::render (view, light);

  view.delMVP (program);

}


