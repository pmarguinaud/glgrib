#include "glgrib_lines.h"
#include "glgrib_program.h"
#include "glgrib_resolve.h"
#include "glgrib_gshhg.h"
#include "glgrib_shapelib.h"

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <stdexcept>

glgrib_lines & glgrib_lines::operator= (const glgrib_lines & other)
{
  clear ();
  if ((this != &other) && other.isReady ())
    {
      glgrib_polygon::operator= (other);
      opts = other.opts;
      setupVertexAttributes ();
      setReady ();
    }
  return *this;
}

void glgrib_lines::setup (const glgrib_options_lines & o, 
                          const std::vector<unsigned int> & mask, 
                          const std::vector<unsigned int> & code)
{

  opts = o;

  numberOfColors = use_alpha () ? 4 : 3;

  std::vector <float> lonlat;
  std::vector <unsigned int> ind;

  if (opts.format == "gshhg")
     glgrib_gshhg::read (opts, &numberOfPoints, &numberOfLines, &lonlat, &ind, mask, code);
  else
     glgrib_shapelib::read (opts, &numberOfPoints, &numberOfLines, &lonlat, &ind, opts.selector);

  vertexbuffer = new_glgrib_opengl_buffer_ptr (lonlat.size () * sizeof (float), lonlat.data ());
  elementbuffer = new_glgrib_opengl_buffer_ptr (ind.size () * sizeof (unsigned int), ind.data ());

  setupVertexAttributes ();

  setReady ();
}

void glgrib_lines::render (const glgrib_view & view, const glgrib_options_light & light) const
{
  glgrib_program * program = glgrib_program_load (glgrib_program::MONO);
  program->use ();
  float color[3] = {(float)opts.color.r / 255.0f, 
                    (float)opts.color.g / 255.0f, 
                    (float)opts.color.b / 255.0f};

  view.setMVP (program);
  program->setLight (light);
  program->set3fv ("color0", color);
  program->set1i ("do_alpha", 1);
  program->set1f ("scale", opts.scale);


  glgrib_polygon::render (view, light);

  view.delMVP (program);

}


