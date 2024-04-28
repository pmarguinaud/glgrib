#include "glGrib/OpenGL.h"
#include "glGrib/Png.h"
#include "glGrib/Program.h"
#include "glGrib/Geometry.h"
#include "glGrib/Font.h"

#include <iostream>
#include <fcntl.h>
#include <unistd.h>

namespace glGrib
{

void glInit ()
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

void OpenGLTexture::init
    (int width, int height, const void * data, GLint internalformat, GLint format)
{
  glGenTextures (1, &id_);
  glBindTexture (GL_TEXTURE_2D, id_);

  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);    
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glTexImage2D (GL_TEXTURE_2D, 0, internalformat, width, height, 
                0, format, GL_UNSIGNED_BYTE, data); 
  glBindTexture (GL_TEXTURE_2D, 0); 
  allocated_ = true;
}

OpenGLTexture::~OpenGLTexture ()
{
  if (allocated_)
    glDeleteTextures (1, &id_);
  allocated_ = false;
}

template <> GLenum getOpenGLType<unsigned char > () { return GL_UNSIGNED_BYTE ; }
template <> GLenum getOpenGLType<unsigned short> () { return GL_UNSIGNED_SHORT; }
template <> GLenum getOpenGLType<unsigned int  > () { return GL_UNSIGNED_INT  ; }
template <> GLenum getOpenGLType<float         > () { return GL_FLOAT         ; }

namespace
{

#ifdef GLGRIB_USE_GLFW
void errorCallback (int c, const char * desc)
{
  fprintf (stderr, "Error: %4d | %s\n", c, desc);
  abort ();
}
#endif

}

#ifdef GLGRIB_USE_EGL
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

eglDisplay * egl = nullptr;

#ifdef GLGRIB_USE_GBM
eglDisplay::eglDisplay 
  (const std::string & path)
{
  fd = open (path.c_str (), O_RDWR);

  if (fd < 0)
    throw std::runtime_error (std::string ("Cannot open ") + path);

  gbm = gbm_create_device (fd);

  if (gbm == nullptr)
    throw std::runtime_error (std::string ("Cannot create gbm object"));

  display = eglGetPlatformDisplay (EGL_PLATFORM_GBM_MESA, gbm, nullptr);
  setup ();
}
#else
eglDisplay::eglDisplay 
  (int idev)
{
  if (idev < 0)
    {
      display = eglGetDisplay (EGL_DEFAULT_DISPLAY); 
    }
  else
    {
      const int mdev = 10;
      EGLDeviceEXT dev[mdev];
      EGLint ndev;
     
      PFNEGLQUERYDEVICESEXTPROC eglQueryDevicesEXT =
        (PFNEGLQUERYDEVICESEXTPROC)
        eglGetProcAddress ("eglQueryDevicesEXT");
     
      eglQueryDevicesEXT (mdev, dev, &ndev);
     
      if (idev >= ndev)
        throw std::runtime_error (std::string ("Device not available"));
     
      PFNEGLGETPLATFORMDISPLAYEXTPROC eglGetPlatformDisplayEXT =
        (PFNEGLGETPLATFORMDISPLAYEXTPROC)
        eglGetProcAddress ("eglGetPlatformDisplayEXT");
     
      display = eglGetPlatformDisplayEXT (EGL_PLATFORM_DEVICE_EXT, dev[idev], nullptr);
    }
  setup ();
}
#endif

void eglDisplay::setup ()
{
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

}

eglDisplay::~eglDisplay ()
{
  if (display)
    eglTerminate (display);
#ifdef GLGRIB_USE_GBM
  if (gbm)
    gbm_device_destroy (gbm);
  if (fd >= 0)
    ::close (fd);
#endif
}

#endif

const OpenGLVersion getOpenGLVersion (float V)
{
  OpenGLVersion version;

  version.major = static_cast<int> (V);

  int v = 1000.0f * (V - static_cast<float> (version.major));

  while (v && (v % 10 == 0))
    v /= 10;

  version.minor = static_cast<int> (v);  

  return version;
}


namespace
{

#define GLMESS(x) case GL_DEBUG_SOURCE_##x: return #x
const char * debugSource (unsigned int source)
{
  switch (source)
    {
      GLMESS (API); GLMESS (WINDOW_SYSTEM); GLMESS (SHADER_COMPILER);
      GLMESS (THIRD_PARTY); GLMESS (APPLICATION); GLMESS (OTHER);
    }
  return "UNKNOWN";
}
#undef GLMESS

#define GLMESS(x) case GL_DEBUG_TYPE_##x: return #x
const char * debugType (unsigned int type)
{
  switch (type)
    {
      GLMESS (ERROR); GLMESS (DEPRECATED_BEHAVIOR); GLMESS (UNDEFINED_BEHAVIOR);
      GLMESS (PORTABILITY); GLMESS (PERFORMANCE); GLMESS (MARKER);
      GLMESS (PUSH_GROUP); GLMESS (POP_GROUP); GLMESS (OTHER);
    }
  return "UNKNOWN";
}
#undef GLMESS

#define GLMESS(x) case GL_DEBUG_SEVERITY_##x: return #x
const char * debugSeverity (unsigned int severity)
{
  switch (severity)
    {
      GLMESS (HIGH); GLMESS (MEDIUM);
      GLMESS (LOW); GLMESS (NOTIFICATION);
    }
  return "UNKNOWN";
}
#undef GLMESS

}

void APIENTRY debugCallback 
  (unsigned int source, unsigned int type, GLuint id, unsigned int severity, 
   int length, const char * message, const void * data)
{
  // ignore non-significant error/warning codes
  if (id == 131169 || id == 131185 || id == 131218 || id == 131204)
    return;
  printf ("%-20s | %-20s | %-30s | %10d | %s\n", debugSource (source),
          debugSeverity (severity), debugType (type), id, message);
}

void glSetupDebug (const OptionsRender & opts)
{
  if (opts.debug.on || opts.info.on)
   {   
#define pp(x) \
     printf ("%-40s : %s\n", #x, glGetString (x));
     pp (GL_VENDOR);
     pp (GL_RENDERER);
     pp (GL_VERSION); 
     pp (GL_SHADING_LANGUAGE_VERSION);
     pp (GL_EXTENSIONS);
#undef pp
   }   
  if (opts.debug.on)
   {   
     GLint flags; 
     glGetIntegerv (GL_CONTEXT_FLAGS, &flags);
     if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
       {
         glEnable (GL_DEBUG_OUTPUT);
         glEnable (GL_DEBUG_OUTPUT_SYNCHRONOUS); 
         glDebugMessageCallback (debugCallback, nullptr);
         glDebugMessageControl (GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
       }
   }   

}

void glStart (const OptionsRender & opts)
{
#ifdef GLGRIB_USE_GLFW
  glfwSetErrorCallback (errorCallback);
  if (! glfwInit ())
    {
      fprintf (stderr, "Failed to initialize GLFW\n");
      exit (1);
    }
#endif
#ifdef GLGRIB_USE_EGL
#ifdef GLGRIB_USE_GBM
  egl = new eglDisplay (opts.egl.gbm.path);
#else
  egl = new eglDisplay (opts.egl.device);
#endif
  glSetupDebug (opts);
#endif
}

void glStop ()
{
  Geometry::clearCache ();
  Font::clearCache ();
  Program::clearCache ();
#ifdef GLGRIB_USE_GLFW
  glfwTerminate ();
#endif
#ifdef GLGRIB_USE_EGL
  delete egl;
  egl = nullptr;
#endif
}

}
