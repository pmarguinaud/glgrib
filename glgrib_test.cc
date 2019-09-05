#include "glgrib_test.h"
#include "glgrib_opengl.h"

#include <stdlib.h>
#include <stdio.h>
#include <iostream>


void glgrib_test::render (const glgrib_view & view, const glgrib_options_light & light) const
{
  glgrib_program * program = glgrib_program_load (glgrib_program::TEST);
  program->use (); 

  view.setMVP (program);

  glBindVertexArray (VertexArrayID);
  glDrawElements (GL_TRIANGLES, 3 * numberOfTriangles, GL_UNSIGNED_INT, NULL);
  glBindVertexArray (0);
}

void glgrib_test::cleanup ()
{
  if (isReady ())
    glDeleteVertexArrays (1, &VertexArrayID);
  glgrib_object::cleanup ();
}

void glgrib_test::init ()
{
  numberOfTriangles = 1;
  numberOfPoints = 3;

  float xyz[] = 
  {
    0., -0.5, -1.,
    0., +1.0, +1.,
    0.,  0.0, +1.
  };
  unsigned int ind[] =
  {
    0, 1, 2
  };

  vertexbuffer = new_glgrib_opengl_buffer_ptr (numberOfPoints * 3 * sizeof (float), xyz);
  elementbuffer = new_glgrib_opengl_buffer_ptr (numberOfTriangles * 3 * sizeof (unsigned int), ind);

  glGenVertexArrays (1, &VertexArrayID);
  glBindVertexArray (VertexArrayID);

  vertexbuffer->bind (GL_ARRAY_BUFFER);
  glEnableVertexAttribArray (0); 
  glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    
  elementbuffer->bind (GL_ELEMENT_ARRAY_BUFFER);
  glBindVertexArray (0); 

  setReady ();
}

