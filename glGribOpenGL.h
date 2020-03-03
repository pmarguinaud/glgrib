#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <memory>


class glGribOpenGLBuffer
{
public:
  glGribOpenGLBuffer (size_t, const void * = nullptr);
  ~glGribOpenGLBuffer ();
  GLuint id () { return id_; }
  bool allocated () const { return allocated_; }
  void bind (GLenum, GLuint = 0) const;
  size_t buffersize () const { return size_; }
  void * map ();
  void unmap ();
private:
  bool allocated_ = false;
  GLuint id_;
  size_t size_;
};

typedef std::shared_ptr<glGribOpenGLBuffer> glGribOpenGLBufferPtr;
extern glGribOpenGLBufferPtr newGlgribOpenGLBufferPtr (size_t, const void * = nullptr);
extern glGribOpenGLBufferPtr newGlgribOpenGLBufferPtr (const glGribOpenGLBufferPtr &);

class glGribOpenGLTexture
{
public:
  glGribOpenGLTexture (int, int, const void *);
  ~glGribOpenGLTexture ();
  GLuint id () { return id_; }
  bool allocated () { return allocated_; }
private:
  bool allocated_ = false;
  GLuint id_;
};

typedef std::shared_ptr<glGribOpenGLTexture> glgrib_opengl_texture_ptr;
extern glgrib_opengl_texture_ptr newGlgribOpenGLTexturePtr (const glgrib_opengl_texture_ptr &);
extern glgrib_opengl_texture_ptr newGlgribOpenGLTexturePtr (int, int, const void *);

void glInit ();

template <typename T> GLenum getOpenGLType ();
template <> GLenum getOpenGLType<unsigned char > ();
template <> GLenum getOpenGLType<unsigned short> ();
template <> GLenum getOpenGLType<unsigned int  > ();
template <> GLenum getOpenGLType<float         > ();

