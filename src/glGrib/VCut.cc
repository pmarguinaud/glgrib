#include "glGrib/VCut.h"
#include "glGrib/Trigonometry.h"
#include "glGrib/OpenGL.h"


namespace glGrib
{

void VCut::render (const View & view, const OptionsLight & light) const
{
  const int Nx = vertexbuffer->size () / 2;
  const int Nz = 3;
  Program * program = Program::load ("VCUT");
  program->use (); 

  view.setMVP (program);
  program->set ("Nx", Nx);
  program->set ("Nz", Nz);

  glDisable (GL_CULL_FACE);
  glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);

  VAID.bind ();
  unsigned int ind[6] = {0, 1, 2, 1, 2, 3};
  glDrawElementsInstanced (GL_TRIANGLES, 6, GL_UNSIGNED_INT, ind, (Nz - 1) * (Nx - 1));
  VAID.unbind ();

  glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
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
