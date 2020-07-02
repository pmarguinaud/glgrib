#include "glGribBatch.h"
#include "glGribOptions.h"
#include "glGribOpenGL.h"
#include "glGribScene.h"
#include "glGribSnapshot.h"
#include "glGribClear.h"

#include <iostream>
#include <stdexcept>

#include <fcntl.h>
#include <unistd.h>
#include <string.h>

namespace
{

int idcount = 0;

#ifdef USE_EGL
bool pre ()
{
  const char * m = nullptr;
  EGLint e = eglGetError (); 
  switch (e)
    {
      case EGL_SUCCESS:
        return true;
      case EGL_NOT_INITIALIZED:
        m = "EGL is not initialized, or could not be initialized, for the specified EGL display connection.";
        break;
      case EGL_BAD_ACCESS:
        m = "EGL cannot access a requested resource (for example a context is bound in another thread).";
        break;
      case EGL_BAD_ALLOC:
        m = "EGL failed to allocate resources for the requested operation.";
        break;
      case EGL_BAD_ATTRIBUTE:
        m = "An unrecognized attribute or attribute value was passed in the attribute list.";
        break;
      case EGL_BAD_CONTEXT:
        m = "An EGLContext argument does not name a valid EGL rendering context.";
        break;
      case EGL_BAD_CONFIG:
        m = "An EGLConfig argument does not name a valid EGL frame buffer configuration.";
        break;
      case EGL_BAD_CURRENT_SURFACE:
        m = "The current surface of the calling thread is a window, pixel buffer or pixmap that is no longer valid.";
        break;
      case EGL_BAD_DISPLAY:
        m = "An EGLDisplay argument does not name a valid EGL display connection.";
        break;
      case EGL_BAD_SURFACE:
        m = "An EGLSurface argument does not name a valid surface (window, pixel buffer or pixmap) configured for GL rendering.";
        break;
      case EGL_BAD_MATCH:
        m = "Arguments are inconsistent (for example, a valid context requires buffers not supplied by a valid surface).";
        break;
      case EGL_BAD_PARAMETER:
        m = "One or more argument values are invalid.";
        break;
      case EGL_BAD_NATIVE_PIXMAP:
        m = "A NativePixmapType argument does not refer to a valid native pixmap.";
        break;
      case EGL_BAD_NATIVE_WINDOW:
        m = "A NativeWindowType argument does not refer to a valid native window.";
        break;
      case EGL_CONTEXT_LOST:
        m = "A power management event has occurred. The application must destroy all contexts and reinitialise OpenGL ES state and objects to continue rendering. ";
        break;
    }
  if (m != nullptr)
    printf ("%s\n", m);

  exit (1);
  return false;
}
#endif

}

glGrib::Batch::eglDisplay::eglDisplay 
  (const std::string & path, int version_major, int version_minor)
{
#ifdef USE_EGL
  fd = open (path.c_str (), O_RDWR);

  if (fd < 0)
    throw std::runtime_error (std::string ("Cannot open ") + path);

  gbm = gbm_create_device (fd);

  if (gbm == nullptr)
    throw std::runtime_error (std::string ("Cannot create gbm object"));

  display = eglGetPlatformDisplay (EGL_PLATFORM_GBM_MESA, gbm, nullptr);
  display || pre ();

  eglInitialize (display, nullptr, nullptr) || pre ();

  const EGLint cfgAttr[] = 
  {
    EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
    EGL_RED_SIZE, 8, EGL_GREEN_SIZE, 8,
    EGL_BLUE_SIZE, 8, EGL_DEPTH_SIZE, 8, EGL_NONE
  };

  const int MAX_NUM_CONFIG = 50;
  EGLint numConfig;
  EGLConfig config[MAX_NUM_CONFIG];

  eglChooseConfig (display, cfgAttr, config, MAX_NUM_CONFIG, &numConfig) || pre ();

  eglBindAPI (EGL_OPENGL_API) || pre ();

  const EGLint ctxAttr[] = 
  {
    EGL_CONTEXT_MAJOR_VERSION, version_major,
    EGL_CONTEXT_MINOR_VERSION, version_minor,
    EGL_NONE
  };

  context = eglCreateContext (display, config[0], EGL_NO_CONTEXT, ctxAttr); 
  context || pre ();

#endif
}


glGrib::Batch::eglDisplay::~eglDisplay ()
{
#ifdef USE_EGL
  if (display)
    eglTerminate (display);
  if (gbm)
    gbm_device_destroy (gbm);
  if (fd >= 0)
    ::close (fd);
#endif
}

glGrib::Batch::Batch (const glGrib::Options & o) : glGrib::Render::Render (o)
{
  setup (o);
}

void glGrib::Batch::setup (const Options & o)
{
  opts = o.render;

  if (egl == nullptr)
    {
      auto version = getOpenGLVersion ();
      egl = std::make_shared<eglDisplay> (opts.device.path, 
                                          version.major, 
                                          version.minor);
    }

  makeCurrent ();

  scene.setup (o);

  reSize (opts.width, opts.height);

  glGrib::glInit ();

  id_ = idcount++;
}

void glGrib::Batch::makeCurrent () 
{
#ifdef USE_EGL
  eglMakeCurrent (egl->display, nullptr, nullptr, egl->context);
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
  clear (scene);
}





