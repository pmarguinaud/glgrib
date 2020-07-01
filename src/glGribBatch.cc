#include "glGribBatch.h"
#include "glGribOptions.h"
#include "glGribOpenGL.h"
#include "glGribScene.h"
#include "glGribSnapshot.h"

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

glGrib::Batch::Batch (const glGrib::Options & o) : glGrib::Render::Render (o)
{
  opts = o.render;

#ifdef USE_EGL
  fd = open (opts.device.path.c_str (), O_RDWR);

  if (fd < 0)
    throw std::runtime_error (std::string ("Cannot open ") + opts.device.path);

  gbm = gbm_create_device (fd);

  if (gbm == nullptr)
    throw std::runtime_error (std::string ("Cannot create gbm object"));

  EGLDisplay display = eglGetPlatformDisplay (EGL_PLATFORM_GBM_MESA, gbm, nullptr);
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

  auto version = getOpenGLVersion ();

  const EGLint ctxAttr[] = 
  {
    EGL_CONTEXT_MAJOR_VERSION, version.major,
    EGL_CONTEXT_MINOR_VERSION, version.minor,
    EGL_NONE
  };

  context = eglCreateContext (display, config[0], EGL_NO_CONTEXT, ctxAttr); 
  context || pre ();

  eglMakeCurrent (display, nullptr, nullptr, context) || pre ();

  scene.setup (o);

  reSize (opts.width, opts.height);

  glGrib::glInit ();

#endif

  id_ = idcount++;
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
  auto opts = getScene ().getOptions ();
  opts.render = this->opts;
  cloned = false;
  return new glGrib::Batch (opts);
}





