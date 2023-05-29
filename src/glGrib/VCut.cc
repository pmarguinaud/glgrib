#include "glGrib/VCut.h"
#include "glGrib/Trigonometry.h"
#include "glGrib/OpenGL.h"


namespace glGrib
{

void VCut::render (const View & view, const OptionsLight & light) const
{
  Program * program = Program::load ("VCUT");
  program->use (); 

  view.setMVP (program);

  glDisable (GL_CULL_FACE);

  VAID.bind ();
  glDrawElements (GL_TRIANGLES, 3 * numberOfTriangles, GL_UNSIGNED_INT, nullptr);
  VAID.unbind ();

  glEnable (GL_CULL_FACE);

  view.delMVP (program);

}

void VCut::setupVertexAttributes () const
{
  vertexbuffer->bind (GL_SHADER_STORAGE_BUFFER, 1);
  elementbuffer->bind (GL_ELEMENT_ARRAY_BUFFER);
}

void VCut::setup ()
{
  std::vector<unsigned int> ind {0, 1, 2};
  std::vector<float> lonlat {0.0f, 0.0f, halfpi, 0.0f, 0.0f, halfpi};

  numberOfTriangles = ind.size () / 3;

  vertexbuffer = OpenGLBufferPtr<float> (lonlat);
  elementbuffer = OpenGLBufferPtr<unsigned int> (ind);

  setReady ();
}

}
