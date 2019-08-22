#include "glgrib_coastlines.h"
#include "glgrib_program.h"
#include "glgrib_resolve.h"
#include "glgrib_gshhg.h"

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <stdexcept>

glgrib_coastlines & glgrib_coastlines::operator= (const glgrib_coastlines & other)
{
  cleanup ();
  if ((this != &other) && other.isReady ())
    {
      glgrib_polygon::operator= (other);
      opts = other.opts;
      setupVertexAttributes ();
      setReady ();
    }
}

void glgrib_coastlines::init (const glgrib_options_coastlines & o)
{

  opts = o;

  numberOfColors = use_alpha () ? 4 : 3;

  std::vector <float> xyz;
  std::vector <unsigned int> ind;

  glgrib_gshhg::read (glgrib_resolve (opts.path), &numberOfPoints, &numberOfLines, &xyz, &ind);


  vertexbuffer = new_glgrib_opengl_buffer_ptr (3 * numberOfPoints * sizeof (float), xyz.data ());
  elementbuffer = new_glgrib_opengl_buffer_ptr (2 * numberOfLines * sizeof (unsigned int), ind.data ());

  setupVertexAttributes ();

  setReady ();
}

void glgrib_coastlines::render (const glgrib_view & view, const glgrib_options_light & light) const
{
  glgrib_program * program = glgrib_program_load (glgrib_program::MONO);
  program->use ();
  float color[3] = {(float)opts.color.r / 255.0f, (float)opts.color.g / 255.0f, (float)opts.color.b / 255.0f};
  view.setMVP (program);
  program->setLight (light);
  program->set3fv ("color0", color);
  program->set1i ("do_alpha", 1);
  program->set1f ("scale", opts.scale);
  glgrib_polygon::render (view, light);
}


