#include "glgrib_test.h"
#include "glgrib_trigonometry.h"
#include "glgrib_opengl.h"

#include <stdlib.h>
#include <stdio.h>



void glgrib_test::render (const glgrib_view & view, const glgrib_options_light & light) const
{
  return;
  glgrib_program * program = glgrib_program::load (glgrib_program::TEST);
  program->use (); 

  view.setMVP (program);

  glDisable (GL_CULL_FACE);

  glBindVertexArray (VertexArrayID);
  glDrawElements (GL_TRIANGLES, 3 * numberOfTriangles, GL_UNSIGNED_INT, nullptr);
  glBindVertexArray (0);

  glEnable (GL_CULL_FACE);

  view.delMVP (program);

}

void glgrib_test::clear ()
{
  if (isReady ())
    glDeleteVertexArrays (1, &VertexArrayID);
  glgrib_object::clear ();
}

void glgrib_test::setup ()
{
  return;
  int numberOfPoints = 3;

  std::vector<unsigned int> ind {0, 1, 2};
  std::vector<float> lonlat {0.0f, 0.0f, halfpi, 0.0f, 0.0f, halfpi};


  numberOfTriangles = ind.size () / 3;

  vertexbuffer = new_glgrib_opengl_buffer_ptr (lonlat.size () * sizeof (lonlat[0]), lonlat.data ());
  elementbuffer = new_glgrib_opengl_buffer_ptr (ind.size () * sizeof (ind[0]), ind.data ());

  glGenVertexArrays (1, &VertexArrayID);
  glBindVertexArray (VertexArrayID);

  vertexbuffer->bind (GL_ARRAY_BUFFER);
  glEnableVertexAttribArray (0); 
  glVertexAttribPointer (0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
    
  elementbuffer->bind (GL_ELEMENT_ARRAY_BUFFER);
  glBindVertexArray (0); 

  setReady ();
}

