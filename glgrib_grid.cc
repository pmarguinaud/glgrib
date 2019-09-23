#include "glgrib_grid.h"
#include "glgrib_program.h"

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <vector>

glgrib_grid & glgrib_grid::operator= (const glgrib_grid & other)
{
  clear ();
  if ((this != &other) && other.isReady ())
    {
      glgrib_object::operator= (other);
      setup (opts);
      setReady ();
    }
}

void glgrib_grid::setup (const glgrib_options_grid & o)
{
  opts = o;

  glGenVertexArrays (1, &VertexArrayID);
  glBindVertexArray (VertexArrayID);

  setReady ();
}

void glgrib_grid::clear ()
{
  if (isReady ()) 
    glDeleteVertexArrays (1, &VertexArrayID);
  glgrib_object::clear (); 
}

glgrib_grid::~glgrib_grid ()
{
  clear (); 
}

void glgrib_grid::render (const glgrib_view & view, const glgrib_options_light & light) const
{
  glgrib_program * program = glgrib_program_load (glgrib_program::GRID);
  program->use ();
  float color[3] = {(float)opts.color.r / 255.0f, 
                    (float)opts.color.g / 255.0f, 
                    (float)opts.color.b / 255.0f};

  const int nn = 100;

  view.setMVP (program);
  program->set3fv ("color0", color);
  program->set1i ("do_alpha", 0);
  program->set1i ("resolution", opts.resolution);
  program->set1f ("scale", opts.scale);
  program->set1i ("nn", 100);

  glBindVertexArray (VertexArrayID);
  glDrawArrays (GL_LINE_STRIP, 0, opts.resolution * (nn + 1));
  glBindVertexArray (0);


}

