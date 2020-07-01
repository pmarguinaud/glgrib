#include "glGribRender.h"
#include "glGribSnapshot.h"


glGrib::Render::Render (const Options & o)
{
}

void glGrib::Render::snapshot (const std::string & format)
{
  glGrib::snapshot (*this, format);
}

void glGrib::Render::framebuffer (const std::string & format)
{
  glGrib::framebuffer (*this, format);
}

void glGrib::Render::reSize (int w, int h)
{
  opts.width = w;
  opts.height = h;
  makeCurrent ();
  glViewport (0, 0, opts.width, opts.height);
  scene.setViewport (opts.width, opts.height);
}

const glGrib::Render::version_t glGrib::Render::getOpenGLVersion () const
{
  version_t version;

  version.major = static_cast<int> (opts.opengl.version);

  int v = 1000.0f * (opts.opengl.version - static_cast<float> (version.major));

  while (v && (v % 10 == 0))
    v /= 10;

  version.minor = static_cast<int> (v);  

  return version;
}

