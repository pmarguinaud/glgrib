#include "glGribBatch.h"
#include "glGribOptions.h"
#include "glGribOpenGL.h"
#include "glGribScene.h"
#include "glGribSnapshot.h"
#include "glGribClear.h"

#include <iostream>
#include <stdexcept>

#include <string.h>

namespace
{

int idcount = 0;

}

glGrib::Batch::Batch (const glGrib::Options & o) : glGrib::Render::Render (o)
{
  setup (o);
  setupDebug ();
}

void glGrib::Batch::setup (const Options & o)
{
  opts = o.render;

  egl = glGrib::egl;

  makeCurrent ();

  scene.setup (o);

  reSize (opts.width, opts.height);

  glGrib::glInit ();

  id_ = idcount++;
}

void glGrib::Batch::makeCurrent () 
{
#ifdef USE_EGL
  eglMakeCurrent (egl->display, nullptr, nullptr, egl->context) || preEGLError ();
  glViewport (0, 0, opts.width, opts.height);
#endif
}

void glGrib::Batch::run (glGrib::Shell * shell)
{
  const auto & opts = getOptions ();

  for (int i = 0; i < opts.offscreen.frames; i++)
    {
      scene.update ();
      framebuffer (opts.offscreen.format);
    }

  close ();
}

void glGrib::Batch::setOptions (const OptionsRender & o) 
{
  if ((o.width != opts.width) || (o.height != opts.height))
    reSize (o.width, o.height);
}


class glGrib::Render * glGrib::Batch::clone () 
{
  cloned = false;

  glGrib::Options opts; 

  opts.render = this->opts;

  Batch * batch = new Batch ();

  batch->egl = egl;

  batch->setup (opts);
  batch->scene = scene;
  
  return batch;
}

glGrib::Batch::~Batch ()
{
// Destroy the scene *before* the EGL context/display is destroyed
  clear ();
}





