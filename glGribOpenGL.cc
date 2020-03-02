#include "glGribOpenGL.h"
#include <iostream>

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
  
void glGribOpenGLBuffer::bind (GLenum target, GLuint index) const 
{
  if (index == 0)
    glBindBuffer (target, id_);
  else
    glBindBufferBase (target, index, id_);
}

glGribOpenGLBuffer::glGribOpenGLBuffer (size_t size, const void * data)
{
  glGenBuffers (1, &id_);
  glBindBuffer (GL_ARRAY_BUFFER, id_);
  glBufferData (GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
  allocated_ = true;
  size_ = size;
}

void * glGribOpenGLBuffer::map ()
{
  bind (GL_ARRAY_BUFFER);
  return glMapBufferRange (GL_ARRAY_BUFFER, 0, size_, GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_FLUSH_EXPLICIT_BIT);
}

void glGribOpenGLBuffer::unmap ()
{
  glFlushMappedBufferRange (GL_ARRAY_BUFFER, 0, size_);
  glUnmapBuffer (GL_ARRAY_BUFFER);
}

glGribOpenGLBuffer::~glGribOpenGLBuffer ()
{
  if (allocated_)
    glDeleteBuffers (1, &id_);
  allocated_ = false;
}

glgrib_opengl_buffer_ptr newGlgribOpenGLBufferPtr (size_t size, const void * data)
{
  return std::make_shared<glGribOpenGLBuffer>(size, data);
}

glgrib_opengl_buffer_ptr newGlgribOpenGLBufferPtr (const glgrib_opengl_buffer_ptr & oldptr)
{
  size_t size = oldptr->buffersize ();
  glgrib_opengl_buffer_ptr newptr = newGlgribOpenGLBufferPtr (size, nullptr);
  oldptr->bind (GL_COPY_READ_BUFFER);
  newptr->bind (GL_COPY_WRITE_BUFFER);

  glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, size);

  glBindBuffer (GL_COPY_WRITE_BUFFER, 0);
  glBindBuffer (GL_COPY_READ_BUFFER, 0);

  return newptr;
}

glGribOpenGLTexture::glGribOpenGLTexture (int width, int height, const void * data)
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

glGribOpenGLTexture::~glGribOpenGLTexture ()
{
  if (allocated_)
    glDeleteTextures (1, &id_);
  allocated_ = false;
}

glgrib_opengl_texture_ptr newGlgribOpenGLTexturePtr (int width, int height, const void * data)
{
  return std::make_shared<glGribOpenGLTexture>(width, height, data);
}

template <> GLenum getOpenGLType<unsigned char > () { return GL_UNSIGNED_BYTE ; }
template <> GLenum getOpenGLType<unsigned short> () { return GL_UNSIGNED_SHORT; }
template <> GLenum getOpenGLType<unsigned int  > () { return GL_UNSIGNED_INT  ; }
template <> GLenum getOpenGLType<float         > () { return GL_FLOAT         ; }
