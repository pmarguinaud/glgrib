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
  
glGrib::OpenGLTexture::OpenGLTexture (int width, int height, const void * data)
{
  glGenTextures (1, &id_);
  glBindTexture (GL_TEXTURE_2D, id_);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  float borderColor[] = {0.0f, 0.0f, 0.0f, 0.0f};
  glTexParameterfv (GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor); 
  glTexImage2D (GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data); 
  glBindTexture (GL_TEXTURE_2D, 0); 
  allocated_ = true;
}

glGrib::OpenGLTexture::~OpenGLTexture ()
{
  if (allocated_)
    glDeleteTextures (1, &id_);
  allocated_ = false;
}

glGrib::OpenGLTexturePtr glGrib::newGlgribOpenGLTexturePtr (int width, int height, const void * data)
{
  return std::make_shared<glGrib::OpenGLTexture>(width, height, data);
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

void glGrib::glfwStart ()
{
  glfwSetErrorCallback (errorCallback);

  if (! glfwInit ())
    {
      fprintf (stderr, "Failed to initialize GLFW\n");
      exit (EXIT_FAILURE);
    }
}

void glGrib::glfwStop ()
{
  glfwTerminate ();
}

