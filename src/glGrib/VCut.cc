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
  unsigned int ind[6] = {0, 1, 2, 1, 2, 3};
  glDrawElementsInstanced (GL_TRIANGLES, 6, GL_UNSIGNED_INT, ind, vertexbuffer->size () / 2 - 1);
  VAID.unbind ();

  glEnable (GL_CULL_FACE);

  view.delMVP (program);

}

void VCut::setupVertexAttributes () const
{
  vertexbuffer->bind (GL_SHADER_STORAGE_BUFFER, 1);
}

void VCut::setup ()
{
  std::vector<float> lonlat {0.0f,  +halfpi, 
	                     0.0f,     0.0f, 
			     0.0f,  -halfpi};

  const int n = 30;
  lonlat.resize (2 * n);

  for (int i = 0; i < n; i++)
    {
      lonlat[2*i+0] = 0.0f;
      lonlat[2*i+1] = -halfpi + pi * static_cast<float> (i) / static_cast<float> (n-1);
    }

  vertexbuffer = OpenGLBufferPtr<float> (lonlat);

  setReady ();
}

}
