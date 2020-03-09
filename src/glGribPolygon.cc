#include "glGribPolygon.h"
#include "glGribOpenGL.h"

#include <stdio.h>

void glGrib::Polygon::setupVertexAttributes ()
{

  glGenVertexArrays (1, &VertexArrayID);
  glBindVertexArray (VertexArrayID);
  
  vertexbuffer->bind (GL_ARRAY_BUFFER);
  glEnableVertexAttribArray (0); 
  glVertexAttribPointer (0, 2, GL_FLOAT, GL_FALSE, 0, nullptr); 

  elementbuffer->bind (GL_ELEMENT_ARRAY_BUFFER);

  glBindVertexArray (0);
}

void glGrib::Polygon::render (const glGrib::View & view, const glGrib::OptionsLight & light) const
{
  glBindVertexArray (VertexArrayID);
  glEnable (GL_PRIMITIVE_RESTART);
  glPrimitiveRestartIndex (0xffffffff);
  glDrawElements (GL_LINE_STRIP, numberOfLines, GL_UNSIGNED_INT, nullptr);
  glDisable (GL_PRIMITIVE_RESTART);
  glBindVertexArray (0);

}

glGrib::Polygon::~Polygon ()
{
  clear ();
}

void glGrib::Polygon::clear ()
{
  if (isReady ())
    glDeleteVertexArrays (1, &VertexArrayID);
  glGrib::Object::clear ();
}
