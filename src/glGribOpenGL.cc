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
  
void glGrib::OpenGLBuffer::bind (GLenum target, GLuint index) const 
{
  if (index == 0)
    glBindBuffer (target, id_);
  else
    glBindBufferBase (target, index, id_);
}

glGrib::OpenGLBuffer::OpenGLBuffer (size_t size, const void * data)
{
  glGenBuffers (1, &id_);

//glNamedBufferData (id_, size, data, GL_STATIC_DRAW); does not work
//glNamedBufferStorage (id_, size, data, GL_MAP_READ_BIT | GL_MAP_WRITE_BIT); does not work

  glBindBuffer (GL_ARRAY_BUFFER, id_);
  glBufferData (GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
  glBindBuffer (GL_ARRAY_BUFFER, 0);

  allocated_ = true;
  size_ = size;
}

glGrib::OpenGLBuffer::~OpenGLBuffer ()
{
  if (allocated_)
    glDeleteBuffers (1, &id_);
  allocated_ = false;
}

glGrib::OpenGLBufferPtr glGrib::newGlgribOpenGLBufferPtr (size_t size, const void * data)
{
  return std::make_shared<glGrib::OpenGLBuffer>(size, data);
}

glGrib::OpenGLBufferPtr glGrib::newGlgribOpenGLBufferPtr (const glGrib::OpenGLBufferPtr & oldptr)
{
  size_t size = oldptr->buffersize ();
  glGrib::OpenGLBufferPtr newptr = glGrib::newGlgribOpenGLBufferPtr (size, nullptr);
  oldptr->bind (GL_COPY_READ_BUFFER);
  newptr->bind (GL_COPY_WRITE_BUFFER);

  glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, size);

  glBindBuffer (GL_COPY_WRITE_BUFFER, 0);
  glBindBuffer (GL_COPY_READ_BUFFER, 0);

  return newptr;
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

