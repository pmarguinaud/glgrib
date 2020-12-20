#include "glGribShellRegular.h"
#include "glGribWindowSet.h"
#include "glGribOptions.h"
#include "glGribBatch.h"
#include "glGribGeometry.h"

#include <iostream>

#define EGL_EGLEXT_PROTOTYPES
#include <EGL/egl.h>
#include <EGL/eglext.h>

#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>

#include <gbm.h>
#include <fcntl.h>
#include <unistd.h>

EGLDisplay display = nullptr;
EGLConfig  config  = nullptr;
EGLContext context = nullptr;
int fd = -1;
struct gbm_device * gbm = nullptr;


static void screenshot_ppm
(const char *filename, unsigned int width,
 unsigned int height, unsigned char *pixels) 
{
  size_t i, j, cur;
  const size_t format_nchannels = 3;
  FILE *f = fopen(filename, "w");
  fprintf(f, "P3\n%d %d\n%d\n", width, height, 255);
  for (i = 0; i < height; i++) {
      for (j = 0; j < width; j++) {
          cur = format_nchannels * ((height - i - 1) * width + j);
          fprintf(f, "%3d %3d %3d ", (pixels)[cur], (pixels)[cur + 1], (pixels)[cur + 2]);
      }
      fprintf(f, "\n");
  }
  fclose(f);
}


static
bool preEGLError ()
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

int main (int argc, const char * argv[])
{
  glGrib::Options opts;

  int fd = open ("/dev/dri/renderD128", O_RDWR);

  if (fd < 0)
    throw std::runtime_error (std::string ("Cannot open /dev/dri/renderD128"));

  gbm = gbm_create_device (fd);

  if (gbm == nullptr)
    throw std::runtime_error (std::string ("Cannot create gbm object"));

  display = eglGetPlatformDisplay (EGL_PLATFORM_GBM_MESA, gbm, nullptr);
  display || preEGLError ();

  eglInitialize (display, nullptr, nullptr) || preEGLError ();

  const EGLint cfgAttr[] = 
  {
    EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
    EGL_RED_SIZE, 8, EGL_GREEN_SIZE, 8,
    EGL_BLUE_SIZE, 8, EGL_DEPTH_SIZE, 8, EGL_NONE
  };

  EGLint numConfig;

  eglChooseConfig (display, cfgAttr, &config, 1, &numConfig) || preEGLError ();

  eglBindAPI (EGL_OPENGL_API) || preEGLError ();

  const EGLint ctxAttr[] = 
  {
    EGL_CONTEXT_MAJOR_VERSION, 4,
    EGL_CONTEXT_MINOR_VERSION, 3,
    EGL_NONE
  };

  context = eglCreateContext (display, config, EGL_NO_CONTEXT, ctxAttr); 
  context || preEGLError ();

  eglMakeCurrent (display, nullptr, nullptr, context) || preEGLError ();

  glGrib::WindowSet * wset = glGrib::WindowSet::create (opts);
  glGrib::ShellRegular & shell = glGrib::ShellRegular::getInstance ();

  if (opts.shell.on)
    {
      shell.setup (opts.shell);
      shell.start (wset);
      if (! shell.isSynchronous ())
        wset->run (&shell);
    }
  else
    {
      wset->run ();
    }

  shell.wait ();

  delete wset;

  eglDestroyContext (display, context) || preEGLError ();

  glGrib::glStop ();

  return 0;
}


