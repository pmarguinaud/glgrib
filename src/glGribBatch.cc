#ifdef GLGRIB_USE_EGL
#include "glGribBatch.h"
#include "glGribOptions.h"
#include "glGribOpenGL.h"
#include "glGribScene.h"
#include "glGribClear.h"
#include "glGribContainer.h"

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
  setup (o, EGL_NO_CONTEXT);
}

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

void Batch::makeCurrent () 
{
  eglMakeCurrent (egl->display, nullptr, nullptr, context) || preEGLError ();
}

void Batch::run (Shell * shell)
{
  const auto & opts = getOptions ();

  const auto & sopts = scene.getOptions ();

  if (sopts.review.on && sopts.review.path->size ())
    {
      auto cont = Container::create (sopts.review.path, true);
      cont->buildIndex ();
      
      std::string e;
    
      while (1)
        {
          e = cont->getNextExt (e);
          if (e == "")
            break;

          auto fopts = sopts.field[0];

          fopts.path.resize (1);
          fopts.path[0] = sopts.review.path + "%" + e;

          scene.setFieldOptions (0, fopts);

          scene.update ();

          framebuffer (opts.offscreen.format);
        }  

    }
  else
    {
      for (int i = 0; i < opts.offscreen.frames; i++)
        {
          scene.update ();
          framebuffer (opts.offscreen.format);
        }
    }

  close ();
}

void Batch::setOptions (const OptionsRender & o) 
{
  if ((o.width != opts.width) || (o.height != opts.height))
    reSize (o.width, o.height);
}


class Render * Batch::clone (bool deep) 
{
  cloned = false;

  Options opts; 

  opts.render = this->opts;

  Batch * batch = new Batch ();

  batch->egl = egl;
  batch->setup (opts, context);

  if (deep)
    batch->scene = scene;
  
  return batch;
}

Batch::~Batch ()
{
// Destroy the scene *before* the EGL context/display is destroyed
  clear ();
  eglDestroyContext (egl->display, context) || preEGLError ();
}

}
#endif


