#include "glGrib/Test.h"
#include "glGrib/Trigonometry.h"
#include "glGrib/OpenGL.h"


namespace glGrib
{

void Test::render (const View & view, const OptionsLight & light) const
{
  return;
  Program * program = Program::load ("TEST");
  program->use (); 

  view.setMVP (program);

  glDisable (GL_CULL_FACE);

  VAID.bind ();
  glDrawElements (GL_TRIANGLES, 3 * numberOfTriangles, GL_UNSIGNED_INT, nullptr);
  VAID.unbind ();

  glEnable (GL_CULL_FACE);

  view.delMVP (program);

}

void Test::setupVertexAttributes () const
{
  vertexbuffer->bind (GL_ARRAY_BUFFER);
  glEnableVertexAttribArray (0); 
  glVertexAttribPointer (0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
  elementbuffer->bind (GL_ELEMENT_ARRAY_BUFFER);
}

void Test::setup ()
{
  return;

  std::vector<unsigned int> ind {0, 1, 2};
  std::vector<float> lonlat {0.0f, 0.0f, halfpi, 0.0f, 0.0f, halfpi};


  numberOfTriangles = ind.size () / 3;

  vertexbuffer = OpenGLBufferPtr<float> (lonlat);
  elementbuffer = OpenGLBufferPtr<unsigned int> (ind);

  setReady ();
}

}
