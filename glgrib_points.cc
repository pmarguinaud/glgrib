#include "glgrib_points.h"
#include "glgrib_bitmap.h"

#include <iostream>
#include <stdlib.h>


glgrib_points::~glgrib_points ()
{
  cleanup ();
}

void glgrib_points::cleanup ()
{
  if (ready)
    glDeleteVertexArrays (1, &VertexArrayID);
}

glgrib_points & glgrib_points::operator= (const glgrib_points & points)
{
  if (this != &points)
    {   
      cleanup (); 
      if (points.ready)
        {
          setupVertexAttributes (); 
          ready = true;
        }
    }   
}

void glgrib_points::setupVertexAttributes ()
{

  // We have no buffer at all, but for some reason, we have to define a vertex array
  // so that the shader work. I do not know why.
  glGenVertexArrays (1, &VertexArrayID);
  glBindVertexArray (VertexArrayID);
  glBindVertexArray (0);

}

void glgrib_points::init (const std::vector<float> &, const std::vector<float> &, const std::vector<float> &)
{
  setupVertexAttributes ();
  ready = true;
}

void glgrib_points::render (const glgrib_view & view) const
{
  if (! ready)
    return;

  glgrib_program * program = glgrib_program_load (glgrib_program::POINTS);
  program->use ();

  view.setMVP (program);
  program->set1f ("x0", 0.1);
  program->set1f ("y0", 0.1);
  program->set1f ("x1", 0.2);
  program->set1f ("y1", 0.2);

  glBindVertexArray (VertexArrayID);

  unsigned int ind[6] = {0, 1, 2, 2, 3, 0};
  glDrawElements (GL_TRIANGLES, 6, GL_UNSIGNED_INT, ind);

  glBindVertexArray (0);

}



