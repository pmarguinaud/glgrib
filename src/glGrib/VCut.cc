#include "glGrib/VCut.h"
#include "glGrib/Trigonometry.h"
#include "glGrib/OpenGL.h"


namespace glGrib
{

void VCut::render (const View & view, const OptionsLight & light) const
{
#ifdef UNDEF
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
#endif
}

void VCut::setupVertexAttributes () const
{
  lonlatbuffer->bind (GL_SHADER_STORAGE_BUFFER, 1);
  valuesbuffer->bind (GL_SHADER_STORAGE_BUFFER, 2);
}

void VCut::setup ()
{
  Nx = 31;
  Nz = 3;

  std::vector<float> lonlat (2 * Nx);

  for (int i = 0; i < Nx / 2; i++)
    {
      lonlat[2*i+0] = 0.0f;
      lonlat[2*i+1] = -halfpi + pi * static_cast<float> (i) / static_cast<float> (Nx-1);
    }

  for (int i = Nx / 2; i < Nx; i++)
    {
      lonlat[2*i+0] = -halfpi + pi * static_cast<float> (i) / static_cast<float> (Nx-1);
      lonlat[2*i+1] = 0.0f;
    }

  lonlatbuffer = OpenGLBufferPtr<float> (lonlat);
  valuesbuffer = OpenGLBufferPtr<float> (Nx * Nz);

  auto values = valuesbuffer->map ();

  for (int iz = 0; iz < Nz; iz++)
    for (int ix = 0; ix < Nx; ix++)
      {
        float x = static_cast<float> (ix) / static_cast<float> (Nx - 1);
        float z = static_cast<float> (iz) / static_cast<float> (Nz - 1);
        values[Nx*iz+ix] = x * z;
      }

  setReady ();
}

}
