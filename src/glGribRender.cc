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

