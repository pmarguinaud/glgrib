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
    {
      glDeleteVertexArrays (1, &VertexArrayID);
    }
}

glgrib_points & glgrib_points::operator= (const glgrib_points & points)
{
  if (this != &points)
    {   
      cleanup (); 
      if (points.ready)
        {
          llsbuffer = points.llsbuffer;
          len = points.len;
          setupVertexAttributes (); 
          ready = true;
        }
    }   
}

void glgrib_points::setupVertexAttributes ()
{
  glGenVertexArrays (1, &VertexArrayID);
  glBindVertexArray (VertexArrayID);
  llsbuffer->bind (GL_ARRAY_BUFFER);
  glEnableVertexAttribArray (0); 
  glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 0, NULL); 
  glVertexAttribDivisor (0, 1); 

  glBindVertexArray (0);

}

void glgrib_points::init (const std::vector<float> & lon, 
                          const std::vector<float> & lat, 
                          const std::vector<float> & siz)
{
  const double deg2rad = M_PI / 180.0;

  len = lon.size ();
  std::vector<float> lls;
  lls.reserve (3 * len);
  for (int i = 0; i < len; i++)
    {
      lls.push_back (lon[i] * deg2rad);
      lls.push_back (lat[i] * deg2rad);
      lls.push_back (siz[i]);
    }
  llsbuffer = new_glgrib_opengl_buffer_ptr (lls.size () * sizeof (float), lls.data ());
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
  glDrawElementsInstanced (GL_TRIANGLES, 6, GL_UNSIGNED_INT, ind, len);

  glBindVertexArray (0);

}



