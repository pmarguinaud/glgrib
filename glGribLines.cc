#include "glGribLines.h"
#include "glGribProgram.h"
#include "glGribResolve.h"
#include "glGribGshhg.h"
#include "glGribShapelib.h"

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <stdexcept>

glGribLines & glGribLines::operator= (const glGribLines & other)
{
  clear ();
  if ((this != &other) && other.isReady ())
    {
      glGribPolygon::operator= (other);
      opts = other.opts;
      setupVertexAttributes ();
      setReady ();
    }
  return *this;
}

void glGribLines::setup (const glGribOptionsLines & o, 
                          const std::vector<unsigned int> & mask, 
                          const std::vector<unsigned int> & code)
{

  opts = o;

  std::vector <float> lonlat;
  std::vector <unsigned int> ind;

  if (opts.format == "gshhg")
     glGribGshhg::read (opts, &numberOfPoints, &numberOfLines, &lonlat, &ind, mask, code);
  else
     glGribShapelib::read (opts, &numberOfPoints, &numberOfLines, &lonlat, &ind, opts.selector);

  vertexbuffer = new_glgrib_opengl_buffer_ptr (lonlat.size () * sizeof (float), lonlat.data ());
  elementbuffer = new_glgrib_opengl_buffer_ptr (ind.size () * sizeof (unsigned int), ind.data ());

  setupVertexAttributes ();

  setReady ();
}

void glGribLines::render (const glGribView & view, const glGribOptionsLight & light) const
{
  glGribProgram * program = glGribProgram::load (glGribProgram::MONO);
  program->use ();
  view.setMVP (program);
  program->set (light);
  program->set ("color0", opts.color);
  program->set ("do_alpha", 1);
  program->set ("scale", opts.scale);

  glGribPolygon::render (view, light);

  view.delMVP (program);

}


