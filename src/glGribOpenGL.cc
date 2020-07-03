#include "glGribOpenGL.h"
#include "glGribPng.h"
#include "glGribGeometry.h"

#include <iostream>
#include <fcntl.h>
#include <unistd.h>

void glGrib::glInit ()
{
  glClearColor (0.0f, 0.0f, 0.0f, 0.0f);
  glEnable (GL_DEPTH_TEST);
  glEnable (GL_BLEND);
  glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glCullFace (GL_BACK);
  glFrontFace (GL_CCW);
  glEnable (GL_CULL_FACE);
  glDepthFunc (GL_LESS); 
  glEnable (GL_MULTISAMPLE);
}
  
void glGrib::OpenGLTexture::init
    (int width, int height, const void * data, GLint internalformat)
{
  glGenTextures (1, &id_);
  glBindTexture (GL_TEXTURE_2D, id_);

  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);    
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glTexImage2D (GL_TEXTURE_2D, 0, internalformat, width, height, 
                0, GL_RGB, GL_UNSIGNED_BYTE, data); 
  glBindTexture (GL_TEXTURE_2D, 0); 
  allocated_ = true;
}

glGrib::OpenGLTexture::~OpenGLTexture ()
{
  if (allocated_)
    glDeleteTextures (1, &id_);
  allocated_ = false;
}

namespace glGrib
{
template <> GLenum getOpenGLType<unsigned char > () { return GL_UNSIGNED_BYTE ; }
template <> GLenum getOpenGLType<unsigned short> () { return GL_UNSIGNED_SHORT; }
template <> GLenum getOpenGLType<unsigned int  > () { return GL_UNSIGNED_INT  ; }
template <> GLenum getOpenGLType<float         > () { return GL_FLOAT         ; }
}

namespace
{

#ifdef USE_GLFW
void errorCallback (int c, const char * desc)
{
  fprintf (stderr, "Error: %4d | %s\n", c, desc);
  abort ();
}
#endif

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

glGrib::eglDisplay * glGrib::egl = nullptr;

glGrib::eglDisplay::eglDisplay 
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


glGrib::eglDisplay::~eglDisplay ()
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

const glGrib::OpenGLVersion glGrib::getOpenGLVersion 
  (const glGrib::OptionsRender & opts) 
{
  OpenGLVersion version;

  version.major = static_cast<int> (opts.opengl.version);

  int v = 1000.0f * (opts.opengl.version - static_cast<float> (version.major));

  while (v && (v % 10 == 0))
    v /= 10;

  version.minor = static_cast<int> (v);  

  return version;
}


void glGrib::glStart (const glGrib::OptionsRender & opts)
{
#ifdef USE_GLFW
  glfwSetErrorCallback (errorCallback);

  if (! glfwInit ())
    {
      fprintf (stderr, "Failed to initialize GLFW\n");
      exit (EXIT_FAILURE);
    }
#endif
#ifdef USE_EGL
  auto version = getOpenGLVersion (opts);
  egl = new glGrib::eglDisplay (opts.device.path, version.major, version.minor);
#endif
}

void glGrib::glStop ()
{
  glGrib::Geometry::clearCache ();
#ifdef USE_GLFW
  glfwTerminate ();
#endif
#ifdef USE_EGL
  delete egl;
  egl = nullptr;
#endif
}


