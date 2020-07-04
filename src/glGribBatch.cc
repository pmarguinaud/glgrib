#include "glGribBatch.h"
#include "glGribOptions.h"
#include "glGribOpenGL.h"
#include "glGribScene.h"
#include "glGribSnapshot.h"
#include "glGribClear.h"

#include <iostream>
#include <stdexcept>

#include <string.h>

namespace glGrib
{

namespace
{

int idcount = 0;

}

Batch::Batch (const Options & o) : Render::Render (o)
{
#ifdef USE_EGL
  setup (o, EGL_NO_CONTEXT);
#endif
}

#ifdef USE_EGL
void Batch::setup (const Options & o, EGLContext c)
{
  opts = o.render;

  egl = glGrib::egl;

  auto version = getOpenGLVersion (opts.opengl.version);

  const EGLint ctxAttr[] = 
  {
    EGL_CONTEXT_MAJOR_VERSION, version.major,
    EGL_CONTEXT_MINOR_VERSION, version.minor,
    EGL_NONE
  };

  context = eglCreateContext (egl->display, egl->config, c, ctxAttr); 
  context || preEGLError ();

  makeCurrent ();

  scene.setup (o);

  reSize (opts.width, opts.height);

  glInit ();

  id_ = idcount++;

}
#endif

void Batch::makeCurrent () 
{
#ifdef USE_EGL
  eglMakeCurrent (egl->display, nullptr, nullptr, context) || preEGLError ();
#endif
}

void Batch::run (Shell * shell)
{
  const auto & opts = getOptions ();

  for (int i = 0; i < opts.offscreen.frames; i++)
    {
      scene.update ();
      framebuffer (opts.offscreen.format);
    }

  close ();
}

void Batch::setOptions (const OptionsRender & o) 
{
  if ((o.width != opts.width) || (o.height != opts.height))
    reSize (o.width, o.height);
}


class Render * Batch::clone () 
{
  cloned = false;

  Options opts; 

  opts.render = this->opts;

  Batch * batch = new Batch ();

#ifdef USE_EGL
  batch->egl = egl;
  batch->setup (opts, context);
#endif
  batch->scene = scene;
  
  return batch;
}

Batch::~Batch ()
{
// Destroy the scene *before* the EGL context/display is destroyed
  clear ();
#ifdef USE_EGL
  eglDestroyContext (egl->display, context) || preEGLError ();
#endif
}

}



