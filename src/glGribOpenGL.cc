#include "glGribOpenGL.h"
#include <iostream>

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

void errorCallback (int c, const char * desc)
{
  fprintf (stderr, "Error: %4d | %s\n", c, desc);
  abort ();
}

}


void glGrib::glStart ()
{
#ifdef USE_GLFW
  glfwSetErrorCallback (errorCallback);

  if (! glfwInit ())
    {
      fprintf (stderr, "Failed to initialize GLFW\n");
      exit (EXIT_FAILURE);
    }
#endif
}

void glGrib::glStop ()
{
#ifdef USE_GLFW
  glfwTerminate ();
#endif
}


