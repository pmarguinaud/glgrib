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

  glGrib::Batch * gwindow = new glGrib::Batch (opts);

  auto & scene = gwindow->getScene ();

  scene.setup (opts);
  glViewport (0, 0, opts.render.width, opts.render.height);
  scene.setViewport (opts.render.width, opts.render.height);

  glClearColor (0.0f, 0.0f, 0.0f, 0.0f);
  glEnable (GL_DEPTH_TEST);
  glEnable (GL_BLEND);
  glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glCullFace (GL_BACK);
  glFrontFace (GL_CCW);
  glEnable (GL_CULL_FACE);
  glDepthFunc (GL_LESS); 
  glEnable (GL_MULTISAMPLE);

//gwindow->framebuffer (opts.render.offscreen.format);
  {
    unsigned int framebufferMMSA;
    unsigned int texturebufferMMSA;
    unsigned int renderbufferMMSA;
    unsigned int framebufferPOST;
    unsigned int texturebufferPOST;

    glGenFramebuffers (1, &framebufferMMSA);
    glBindFramebuffer (GL_FRAMEBUFFER, framebufferMMSA);

    glGenTextures (1, &texturebufferMMSA);
    glBindTexture (GL_TEXTURE_2D_MULTISAMPLE, texturebufferMMSA);
    glTexImage2DMultisample (GL_TEXTURE_2D_MULTISAMPLE, opts.render.antialiasing.samples, 
      	               GL_RGB, opts.render.width, opts.render.height, GL_TRUE);
    glBindTexture (GL_TEXTURE_2D_MULTISAMPLE, 0);
    glFramebufferTexture2D (GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
                            GL_TEXTURE_2D_MULTISAMPLE, texturebufferMMSA, 0);

    glGenRenderbuffers (1, &renderbufferMMSA);
    glBindRenderbuffer (GL_RENDERBUFFER, renderbufferMMSA);
    glRenderbufferStorageMultisample (GL_RENDERBUFFER, opts.render.antialiasing.samples, 
      	                        GL_DEPTH24_STENCIL8, opts.render.width, opts.render.height);
    glBindRenderbuffer (GL_RENDERBUFFER, 0);
    glFramebufferRenderbuffer (GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, 
                               GL_RENDERBUFFER, renderbufferMMSA);

    if (glCheckFramebufferStatus (GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
      throw std::runtime_error (std::string ("Framebuffer is not complete"));

    glGenFramebuffers (1, &framebufferPOST);
    glBindFramebuffer (GL_FRAMEBUFFER, framebufferPOST);

    glGenTextures (1, &texturebufferPOST);
    glBindTexture (GL_TEXTURE_2D, texturebufferPOST);
    glTexImage2D (GL_TEXTURE_2D, 0, GL_RGB, opts.render.width, opts.render.height, 
                  0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D (GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
                            GL_TEXTURE_2D, texturebufferPOST, 0);	

    if (glCheckFramebufferStatus (GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
      throw std::runtime_error (std::string ("Framebuffer is not complete"));

    glBindFramebuffer (GL_FRAMEBUFFER, framebufferMMSA);
    
    scene.render ();

    glBindFramebuffer (GL_READ_FRAMEBUFFER, framebufferMMSA);
    glBindFramebuffer (GL_DRAW_FRAMEBUFFER, framebufferPOST);
    glBlitFramebuffer (0, 0, opts.render.width, opts.render.height, 0, 0, opts.render.width, opts.render.height, 
                       GL_COLOR_BUFFER_BIT, GL_NEAREST);

    glBindFramebuffer (GL_FRAMEBUFFER, framebufferPOST);
    
    gwindow->snapshot (opts.render.offscreen.format);

    glDeleteTextures (1, &texturebufferPOST);
    glDeleteFramebuffers (1, &framebufferPOST);
    glDeleteRenderbuffers (1, &renderbufferMMSA);
    glDeleteTextures (1, &texturebufferMMSA);
    glDeleteFramebuffers (1, &framebufferMMSA);
    
    glBindFramebuffer (GL_FRAMEBUFFER, 0);
  }

  delete gwindow;

  eglDestroyContext (display, context) || preEGLError ();

  glGrib::glStop ();

  return 0;
}


