#include "glGrib/Polygon.h"
#include "glGrib/OpenGL.h"

namespace glGrib
{

void Polygon::setupVertexAttributes () const
{
  vertexbuffer->bind (GL_ARRAY_BUFFER);
  glEnableVertexAttribArray (0); 
  glVertexAttribPointer (0, 2, GL_FLOAT, GL_FALSE, 0, nullptr); 
  elementbuffer->bind (GL_ELEMENT_ARRAY_BUFFER);
}

void Polygon::render (const View & view, const OptionsLight & light) const
{
  VAID.bind ();
  glEnable (GL_PRIMITIVE_RESTART);
  glPrimitiveRestartIndex (OpenGL::restart);
  glDrawElements (GL_LINE_STRIP, getNumberOfLines (), GL_UNSIGNED_INT, nullptr);
  glDisable (GL_PRIMITIVE_RESTART);
  VAID.unbind ();
}

}
