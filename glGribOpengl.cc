#include "glGribOpengl.h"
#include <iostream>

void gl_init ()
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
  
void glGribOpenglBuffer::bind (GLenum target, GLuint index) const 
{
  if (index == 0)
    glBindBuffer (target, id_);
  else
    glBindBufferBase (target, index, id_);
}

glGribOpenglBuffer::glGribOpenglBuffer (size_t size, const void * data)
{
  glGenBuffers (1, &id_);
  glBindBuffer (GL_ARRAY_BUFFER, id_);
  glBufferData (GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
  allocated_ = true;
  size_ = size;
}

void * glGribOpenglBuffer::map ()
{
  bind (GL_ARRAY_BUFFER);
  return glMapBufferRange (GL_ARRAY_BUFFER, 0, size_, GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_FLUSH_EXPLICIT_BIT);
}

void glGribOpenglBuffer::unmap ()
{
  glFlushMappedBufferRange (GL_ARRAY_BUFFER, 0, size_);
  glUnmapBuffer (GL_ARRAY_BUFFER);
}

glGribOpenglBuffer::~glGribOpenglBuffer ()
{
  if (allocated_)
    glDeleteBuffers (1, &id_);
  allocated_ = false;
}

glgrib_opengl_buffer_ptr newGlgribOpenglBufferPtr (size_t size, const void * data)
{
  return std::make_shared<glGribOpenglBuffer>(size, data);
}

glgrib_opengl_buffer_ptr newGlgribOpenglBufferPtr (const glgrib_opengl_buffer_ptr & oldptr)
{
  size_t size = oldptr->buffersize ();
  glgrib_opengl_buffer_ptr newptr = newGlgribOpenglBufferPtr (size, nullptr);
  oldptr->bind (GL_COPY_READ_BUFFER);
  newptr->bind (GL_COPY_WRITE_BUFFER);

  glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, size);

  glBindBuffer (GL_COPY_WRITE_BUFFER, 0);
  glBindBuffer (GL_COPY_READ_BUFFER, 0);

  return newptr;
}

glGribOpenglTexture::glGribOpenglTexture (int width, int height, const void * data)
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

glGribOpenglTexture::~glGribOpenglTexture ()
{
  if (allocated_)
    glDeleteTextures (1, &id_);
  allocated_ = false;
}

glgrib_opengl_texture_ptr newGlgribOpenglTexturePtr (int width, int height, const void * data)
{
  return std::make_shared<glGribOpenglTexture>(width, height, data);
}

template <> GLenum getOpenglType<unsigned char > () { return GL_UNSIGNED_BYTE ; }
template <> GLenum getOpenglType<unsigned short> () { return GL_UNSIGNED_SHORT; }
template <> GLenum getOpenglType<unsigned int  > () { return GL_UNSIGNED_INT  ; }
template <> GLenum getOpenglType<float         > () { return GL_FLOAT         ; }
