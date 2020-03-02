#include "glGribPolygon.h"
#include "glGribOpenGL.h"

#include <stdio.h>

void glGribPolygon::setupVertexAttributes ()
{

  glGenVertexArrays (1, &VertexArrayID);
  glBindVertexArray (VertexArrayID);
  
  vertexbuffer->bind (GL_ARRAY_BUFFER);
  glEnableVertexAttribArray (0); 
  glVertexAttribPointer (0, 2, GL_FLOAT, GL_FALSE, 0, nullptr); 

  elementbuffer->bind (GL_ELEMENT_ARRAY_BUFFER);

  glBindVertexArray (0);
}

void glGribPolygon::render (const glGribView & view, const glGribOptionsLight & light) const
{
  glBindVertexArray (VertexArrayID);
  glEnable (GL_PRIMITIVE_RESTART);
  glPrimitiveRestartIndex (0xffffffff);
  glDrawElements (GL_LINE_STRIP, numberOfLines, GL_UNSIGNED_INT, nullptr);
  glDisable (GL_PRIMITIVE_RESTART);
  glBindVertexArray (0);

}

glGribPolygon::~glGribPolygon ()
{
  clear ();
}

void glGribPolygon::clear ()
{
  if (isReady ())
    glDeleteVertexArrays (1, &VertexArrayID);
  glGribObject::clear ();
}
