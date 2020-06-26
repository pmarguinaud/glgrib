#include <omp.h>

#include "glGribOptions.h"
#include "glGribOpenGL.h"
#include "glGribScene.h"

#include <iostream>

#include <assert.h>
#include <fcntl.h>
#include <gbm.h>
#include <unistd.h>
#include <string.h>

namespace
{

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

void screenshot_ppm
(const char *filename, unsigned int width,
 unsigned int height, unsigned char *pixels)
{
  const size_t format_nchannels = 3;
  FILE * fp = fopen (filename, "w");
  fprintf (fp, "P3\n%d %d\n%d\n", width, height, 255);
  for (int i = 0; i < static_cast<int> (height); i++) 
    {
      for (int j = 0; j < static_cast<int> (width); j++) 
        {
          int cur = format_nchannels * ((height - i - 1) * width + j);
          fprintf (fp, "%3d %3d %3d ", (pixels)[cur], (pixels)[cur + 1], (pixels)[cur + 2]);
        }
      fprintf(fp, "\n");
    }
  fclose (fp);
}

EGLDisplay getEGLDisplay (const int width, const int height)
{
  int fd = open ("/dev/dri/renderD128", O_RDWR);
  assert (fd > 0);

  struct gbm_device * gbm = gbm_create_device (fd);
  assert (gbm != nullptr);

  EGLDisplay display = eglGetPlatformDisplay(EGL_PLATFORM_GBM_MESA, gbm, nullptr);
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
    EGL_CONTEXT_MAJOR_VERSION, 4,
    EGL_CONTEXT_MINOR_VERSION, 3,
    EGL_NONE
  };

  EGLContext context = eglCreateContext (display, config[0], EGL_NO_CONTEXT, ctxAttr); 
  context || pre ();

  eglMakeCurrent (display, nullptr, nullptr, context) || pre ();

  return display;
}

}

int main (int argc, const char * argv[])
{
  glGrib::Options opts;

  if (! opts.parse (argc, argv))
    return 1;

  const int width = opts.window.width, height = opts.window.height;

  EGLDisplay display = getEGLDisplay (width, height);

  // Create framebuffer for rendering

  GLuint fbo;
  GLuint rbo_color;
  GLuint rbo_depth;

  glGenFramebuffers (1, &fbo);
  glBindFramebuffer (GL_FRAMEBUFFER, fbo);

  glGenRenderbuffers (1, &rbo_color);
  glBindRenderbuffer (GL_RENDERBUFFER, rbo_color);
  /* Storage must be one of: */
  /* GL_RGBA4, GL_RGB565, GL_RGB5_A1, GL_DEPTH_COMPONENT16, GL_STENCIL_INDEX8. */
  glRenderbufferStorage (GL_RENDERBUFFER, GL_RGB565, width, height);
  glFramebufferRenderbuffer (GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                             GL_RENDERBUFFER, rbo_color);

  glGenRenderbuffers (1, &rbo_depth);
  glBindRenderbuffer (GL_RENDERBUFFER, rbo_depth);
  glRenderbufferStorage (GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, width, height);
  glFramebufferRenderbuffer (GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                             GL_RENDERBUFFER, rbo_depth);


  glReadBuffer (GL_COLOR_ATTACHMENT0);
  glPixelStorei (GL_UNPACK_ALIGNMENT, 1);

  glGrib::glInit ();

  glViewport (0, 0, width, height);

  // Render

//glClearColor (0.0f, 1.0f, 0.0f, 0.0f);
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glGrib::Scene scene;

  scene.setup (opts);
  scene.setViewport (width, height);
  scene.update ();
  scene.render ();

  glFlush ();


  // Write image to disk
  unsigned char * pixels = new unsigned char[3 * width * height];

  std::fill (pixels, pixels + 3 * width * height, 0);

  glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels);

  screenshot_ppm ("toto.ppm", width, height, pixels);
  free (pixels);

  // Cleanup
  glDeleteFramebuffers (1, &fbo);
  glDeleteRenderbuffers (1, &rbo_color);
  glDeleteRenderbuffers (1, &rbo_depth);

  eglTerminate (display);

  return 0;
}


