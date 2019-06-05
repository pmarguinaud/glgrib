#include "glgrib_polygon.h"
#include "glgrib_opengl.h"

#include <stdio.h>

void glgrib_polygon::setupVertexAttributes ()
{

  glGenVertexArrays (1, &VertexArrayID);
  glBindVertexArray (VertexArrayID);
  
  vertexbuffer->bind (GL_ARRAY_BUFFER);
  glEnableVertexAttribArray (0); 
  glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 0, NULL); 

  elementbuffer->bind (GL_ELEMENT_ARRAY_BUFFER);

  glBindVertexArray (0);
}

void glgrib_polygon::render (const glgrib_view & view, const glgrib_options_light & light) const
{
  glBindVertexArray (VertexArrayID);
  glDrawElements (GL_LINES, 2 * numberOfLines, GL_UNSIGNED_INT, NULL);
  glBindVertexArray (0);
}

glgrib_polygon::~glgrib_polygon ()
{
  cleanup ();
}

void glgrib_polygon::cleanup ()
{
  if (isReady ())
    glDeleteVertexArrays (1, &VertexArrayID);
  glgrib_object::cleanup ();
}
